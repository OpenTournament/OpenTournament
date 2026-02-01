// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_TeamCreationComponent.h"

#include "Engine/World.h"

#include "GameModes/UR_ExperienceManagerComponent.h"
#include "UR_TeamPublicInfo.h"
#include "UR_TeamPrivateInfo.h"
#include "Player/UR_PlayerState.h"
#include "GameModes/UR_GameMode.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_TeamCreationComponent)

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_TeamCreationComponent::UUR_TeamCreationComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PublicTeamInfoClass = AUR_TeamPublicInfo::StaticClass();
    PrivateTeamInfoClass = AUR_TeamPrivateInfo::StaticClass();
}

#if WITH_EDITOR
EDataValidationResult UUR_TeamCreationComponent::IsDataValid(FDataValidationContext& Context) const
{
    EDataValidationResult Result = Super::IsDataValid(Context);

    //@TODO: TEAMS: Validate that all display assets have the same properties set!

    return Result;
}
#endif

void UUR_TeamCreationComponent::BeginPlay()
{
    Super::BeginPlay();

    // Listen for the experience load to complete
    AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
    UUR_ExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UUR_ExperienceManagerComponent>();
    check(ExperienceComponent);
    ExperienceComponent->CallOrRegister_OnExperienceLoaded_HighPriority(FOnGameExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
}

void UUR_TeamCreationComponent::OnExperienceLoaded(const UUR_ExperienceDefinition* Experience)
{
#if WITH_SERVER_CODE
    if (HasAuthority())
    {
        ServerCreateTeams();
        ServerAssignPlayersToTeams();
    }
#endif
}

#if WITH_SERVER_CODE

void UUR_TeamCreationComponent::ServerCreateTeams()
{
    for (const auto& KVP : TeamsToCreate)
    {
        const int32 TeamId = KVP.Key;
        ServerCreateTeam(TeamId, KVP.Value);
    }
}

void UUR_TeamCreationComponent::ServerAssignPlayersToTeams()
{
    // Assign players that already exist to teams
    AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
    for (APlayerState* PS : GameState->PlayerArray)
    {
        if (AUR_PlayerState* GamePS = Cast<AUR_PlayerState>(PS))
        {
            ServerChooseTeamForPlayer(GamePS);
        }
    }

    // Listen for new players logging in
    AUR_GameMode* GameMode = Cast<AUR_GameMode>(GameState->AuthorityGameMode);
    check(GameMode);

    GameMode->OnGameModePlayerInitialized.AddUObject(this, &ThisClass::OnPlayerInitialized);
}

void UUR_TeamCreationComponent::ServerChooseTeamForPlayer(AUR_PlayerState* PS)
{
    if (PS->IsOnlyASpectator())
    {
        PS->SetGenericTeamId(FGenericTeamId::NoTeam);
    }
    else
    {
        const FGenericTeamId TeamID = IntegerToGenericTeamId(GetLeastPopulatedTeamID());
        PS->SetGenericTeamId(TeamID);
    }
}

void UUR_TeamCreationComponent::OnPlayerInitialized(AGameModeBase* GameMode, AController* NewPlayer)
{
    check(NewPlayer);
    check(NewPlayer->PlayerState);
    if (AUR_PlayerState* PS = Cast<AUR_PlayerState>(NewPlayer->PlayerState))
    {
        ServerChooseTeamForPlayer(PS);
    }
}

void UUR_TeamCreationComponent::ServerCreateTeam(int32 TeamId, UUR_TeamDisplayAsset* DisplayAsset)
{
    check(HasAuthority());

    //@TODO: ensure the team doesn't already exist

    UWorld* World = GetWorld();
    check(World);

    FActorSpawnParameters SpawnInfo;
    SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AUR_TeamPublicInfo* NewTeamPublicInfo = World->SpawnActor<AUR_TeamPublicInfo>(PublicTeamInfoClass, SpawnInfo);
    checkf(NewTeamPublicInfo != nullptr, TEXT("Failed to create public team actor from class %s"), *GetPathNameSafe(*PublicTeamInfoClass));
    NewTeamPublicInfo->SetTeamId(TeamId);
    NewTeamPublicInfo->SetTeamDisplayAsset(DisplayAsset);

    AUR_TeamPrivateInfo* NewTeamPrivateInfo = World->SpawnActor<AUR_TeamPrivateInfo>(PrivateTeamInfoClass, SpawnInfo);
    checkf(NewTeamPrivateInfo != nullptr, TEXT("Failed to create private team actor from class %s"), *GetPathNameSafe(*PrivateTeamInfoClass));
    NewTeamPrivateInfo->SetTeamId(TeamId);
}

int32 UUR_TeamCreationComponent::GetLeastPopulatedTeamID() const
{
    const int32 NumTeams = TeamsToCreate.Num();
    if (NumTeams > 0)
    {
        TMap<int32, uint32> TeamMemberCounts;
        TeamMemberCounts.Reserve(NumTeams);

        for (const auto& KVP : TeamsToCreate)
        {
            const int32 TeamId = KVP.Key;
            TeamMemberCounts.Add(TeamId, 0);
        }

        AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
        for (APlayerState* PS : GameState->PlayerArray)
        {
            if (AUR_PlayerState* GamePS = Cast<AUR_PlayerState>(PS))
            {
                const int32 PlayerTeamID = GamePS->GetTeamId();

                if ((PlayerTeamID != INDEX_NONE) && !PS->IsInactive())	// do not count unassigned or disconnected players
                {
                    check(TeamMemberCounts.Contains(PlayerTeamID))
                    TeamMemberCounts[PlayerTeamID] += 1;
                }
            }
        }

        // sort by lowest team population, then by team ID
        int32 BestTeamId = INDEX_NONE;
        uint32 BestPlayerCount = TNumericLimits<uint32>::Max();
        for (const auto& KVP : TeamMemberCounts)
        {
            const int32 TestTeamId = KVP.Key;
            const uint32 TestTeamPlayerCount = KVP.Value;

            if (TestTeamPlayerCount < BestPlayerCount)
            {
                BestTeamId = TestTeamId;
                BestPlayerCount = TestTeamPlayerCount;
            }
            else if (TestTeamPlayerCount == BestPlayerCount)
            {
                if ((TestTeamId < BestTeamId) || (BestTeamId == INDEX_NONE))
                {
                    BestTeamId = TestTeamId;
                    BestPlayerCount = TestTeamPlayerCount;
                }
            }
        }

        return BestTeamId;
    }

    return INDEX_NONE;
}
#endif	// WITH_SERVER_CODE
