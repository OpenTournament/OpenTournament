// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_PlayerState.h"

//#include "Net/Core/PushModel/PushModel.h"
#include <AbilitySystemComponent.h>
#include <Components/GameFrameworkComponentManager.h>
#include <Kismet/KismetSystemLibrary.h>
#include <Net/UnrealNetwork.h>

#include "UR_AbilitySet.h"
#include "UR_AbilitySystemComponent.h"
#include "UR_Character.h"
#include "UR_ExperienceManagerComponent.h"
#include "UR_FunctionLibrary.h"
#include "UR_GameMode.h"
#include "UR_GameplayTags.h"
#include "UR_GameState.h"
#include "UR_LogChannels.h"
#include "UR_MPC_Global.h"
#include "UR_PawnData.h"
#include "UR_TeamInfo.h"
#include "UR_UserSettings.h"
#include "Attributes/UR_CombatSet.h"
#include "Attributes/UR_HealthSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_PlayerState)

/////////////////////////////////////////////////////////////////////////////////////////////////

const FName AUR_PlayerState::NAME_GameAbilityReady("GameAbilitiesReady");

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_PlayerState::AUR_PlayerState(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    TeamIndex = -1;
    ReplicatedTeamIndex = -1;

    OnPawnSet.AddUniqueDynamic(this, &ThisClass::InternalOnPawnSet);
    OnTeamChanged.AddUniqueDynamic(this, &ThisClass::InternalOnTeamChanged);

    //

    AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UUR_AbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    // These attribute sets will be detected by AbilitySystemComponent::InitializeComponent. Keeping a reference so that the sets don't get garbage collected before that.
    HealthSet = CreateDefaultSubobject<UUR_HealthSet>(TEXT("HealthSet"));
    CombatSet = CreateDefaultSubobject<UUR_CombatSet>(TEXT("CombatSet"));

    // AbilitySystemComponent needs to be updated at a high frequency.
    SetNetUpdateFrequency(100.0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    FDoRepLifetimeParams SharedParams;
    SharedParams.bIsPushBased = true;

    DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Kills, SharedParams);
    DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Deaths, SharedParams);
    DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Suicides, SharedParams);

    DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, CharacterCustomization, SharedParams);

    SharedParams.RepNotifyCondition = REPNOTIFY_OnChanged;
    DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ReplicatedTeamIndex, SharedParams);


    DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PawnData, SharedParams);
    //DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MyTeamID, SharedParams);
    //DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MySquadID, SharedParams);

    SharedParams.Condition = ELifetimeCondition::COND_SkipOwner;
    //DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ReplicatedViewRotation, SharedParams);

    DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, StatTags, SharedParams);
}

void AUR_PlayerState::PreInitializeComponents()
{
    Super::PreInitializeComponents();
}

void AUR_PlayerState::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    check(AbilitySystemComponent);
    AbilitySystemComponent->InitAbilityActorInfo(this, GetPawn());

    const UWorld* World = GetWorld();
    if (World && World->IsGameWorld() && World->GetNetMode() != NM_Client)
    {
        const AGameStateBase* GameState = GetWorld()->GetGameState();
        check(GameState);
        UUR_ExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UUR_ExperienceManagerComponent>();
        check(ExperienceComponent);
        ExperienceComponent->CallOrRegister_OnExperienceLoaded(FOnGameExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
    }
}


void AUR_PlayerState::BeginPlay()
{
    Super::BeginPlay();

    // NOTE: Don't know if PlayerState can actually replicate before PC
    if (const auto PC = GetOwner<APlayerController>())
    {
        if (PC->IsLocalController())
        {
            // Not sure what's the best place to do client->server replication of user customization.
            // We want to do this as early as possible.
            // We may have to delay the first spawning of a player until we are roughly certain that PlayerState & customization has already replicated to everyone.
            // Otherwise we'll have ugly situation where we snap-in customization onto the spawned character.
            ServerSetCharacterCustomization(UUR_UserSettings::Get(this)->CharacterCustomization);
        }
    }
    else if (HasAuthority())
    {
        // Bot = make random customization
        ServerSetCharacterCustomization(UUR_CharacterCustomizationBackend::MakeRandomCharacterCustomization());
    }
}

UAbilitySystemComponent* AUR_PlayerState::GetAbilitySystemComponent() const
{
    return GetGameAbilitySystemComponent();
}

void AUR_PlayerState::OnExperienceLoaded(const UUR_ExperienceDefinition* /*CurrentExperience*/)
{
    if (AUR_GameMode* UR_GameMode = GetWorld()->GetAuthGameMode<AUR_GameMode>())
    {
        if (const UUR_PawnData* NewPawnData = UR_GameMode->GetPawnDataForController(GetOwningController()))
        {
            SetPawnData(NewPawnData);
        }
        else
        {
            UE_LOG(LogGame, Error, TEXT("AUR_PlayerState::OnExperienceLoaded(): Unable to find PawnData to initialize player state [%s]!"), *GetNameSafe(this));
        }
    }
}

void AUR_PlayerState::OnRep_PawnData()
{
    // Noop
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_PlayerState::RegisterKill(AController* Victim, FGameplayTagContainer& OutTags)
{
    // @! TODO : Probably deprecate this behavior here

    // TODO: might want to check teams here.
    // Although including teamkills in multikills & sprees might not be a bad thing.

    Kills++;
    //MARK_PROPERTY_DIRTY_FROM_NAME(AUR_PlayerState, Kills, this);

    if (GetWorld()->TimeSince(LastKillTime) < 3.f)
    {
        MultiKillCount++;
    }
    else
    {
        MultiKillCount = 1;
    }
    LastKillTime = GetWorld()->TimeSeconds;

    if (MultiKillCount > 1)
    {
        // @! TODO: Deprecate
        static TArray<FGameplayTag> MultiKillTags =
        {
            URGameplayTags::Announcement_Accolade_EliminationChord_2x,
            URGameplayTags::Announcement_Accolade_EliminationChord_3x,
            URGameplayTags::Announcement_Accolade_EliminationChord_4x,
            URGameplayTags::Announcement_Accolade_EliminationChord_5x,
            URGameplayTags::Announcement_Accolade_EliminationChord_6x
        };
        OutTags.AddTag(MultiKillTags[FMath::Min(MultiKillCount - 2, MultiKillTags.Num() - 1)]);
    }

    SpreeCount++;
    if ((SpreeCount % 5) == 0)
    {
        SpreeLevel = SpreeCount / 5;

        // @! TODO: Deprecate
        static TArray<FGameplayTag> SpreeTags =
        {
            URGameplayTags::Announcement_Accolade_EliminationStreak_5,
            URGameplayTags::Announcement_Accolade_EliminationStreak_10,
            URGameplayTags::Announcement_Accolade_EliminationStreak_15,
            URGameplayTags::Announcement_Accolade_EliminationStreak_20,
            URGameplayTags::Announcement_Accolade_EliminationStreak_25,
            URGameplayTags::Announcement_Accolade_EliminationStreak_30,
        };
        OutTags.AddTag(SpreeTags[FMath::Min(SpreeLevel - 1, SpreeTags.Num() - 1)]);
    }

    if (LastKiller && Victim && Victim->GetPawn() == LastKiller)
    {
        // @! TODO: Deprecate
        OutTags.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("Announcement.Reward.Kill.Revenge"))));
    }

    ForceNetUpdate();
}

void AUR_PlayerState::RegisterDeath(AController* Killer, FGameplayTagContainer& OutTags)
{
    Deaths++;
    //MARK_PROPERTY_DIRTY_FROM_NAME(AUR_PlayerState, Deaths, this);

    if (SpreeLevel > 0)
    {
        OutTags.AddTag(URGameplayTags::Announcement_Accolade_EliminationStreak_Ended);
    }

    SpreeLevel = 0;
    SpreeCount = 0;
    MultiKillCount = 0;
    LastKillTime = -10;

    if (Killer)
    {
        LastKiller = Killer->GetPawn();
    }

    ForceNetUpdate();
}

void AUR_PlayerState::RegisterSuicide(FGameplayTagContainer& OutExtras)
{
    Suicides++;
    //MARK_PROPERTY_DIRTY_FROM_NAME(AUR_PlayerState, Suicides, this);
    ForceNetUpdate();
}

void AUR_PlayerState::AddScore(const int32 Value)
{
    SetScore(GetScore() + Value);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

int32 AUR_PlayerState::GetTeamIndex_Implementation()
{
    return TeamIndex;
}

void AUR_PlayerState::SetTeamIndex_Implementation(int32 NewTeamIndex)
{
    if (NewTeamIndex != TeamIndex)
    {
        int32 OldTeamIndex = TeamIndex;
        AUR_TeamInfo* OldTeam = Team;
        if (OldTeam)
        {
            if (OldTeam->Players.Remove(this) > 0)
            {
                // team left event
                OldTeam->OnPlayerLeftTeam.Broadcast(OldTeam, this);
            }
        }

        TeamIndex = NewTeamIndex;

        if (HasAuthority())
        {
            ReplicatedTeamIndex = TeamIndex;
            //MARK_PROPERTY_DIRTY_FROM_NAME(AUR_PlayerState, ReplicatedTeamIndex, this);
            ForceNetUpdate();
        }

        Team = AUR_TeamInfo::GetTeamFromIndex(this, TeamIndex);

        if (Team)
        {
            if (Team->Players.AddUnique(this) >= 0)
            {
                // team joined event
                Team->OnPlayerJoinedTeam.Broadcast(Team, this);
            }
        }

        if (AUR_GameState* GS = GetWorld()->GetGameState<AUR_GameState>())
        {
            GS->TrimTeams();
        }

        // team changed event
        OnTeamChanged.Broadcast(this, OldTeamIndex, TeamIndex);
    }
}

void AUR_PlayerState::OnRep_ReplicatedTeamIndex()
{
    if (!HasAuthority() && ReplicatedTeamIndex != TeamIndex)
    {
        //WARNING: it is very possible to receive ReplicatedTeamIndex before receiving the corresponding TeamInfo actor
        if (ReplicatedTeamIndex >= 0 && !AUR_TeamInfo::GetTeamFromIndex(this, ReplicatedTeamIndex))
        {
            return; // We will catch up from UR_TeamInfo::BeginPlay
        }

        // Maintains all the Team->Players arrays on client-side & trigger events appropriately.
        IUR_TeamInterface::Execute_SetTeamIndex(this, ReplicatedTeamIndex);
    }
}

void AUR_PlayerState::SetPawnData(const UUR_PawnData* InPawnData)
{
    check(InPawnData);

    if (GetLocalRole() != ROLE_Authority)
    {
        return;
    }

    if (PawnData)
    {
        UE_LOG(LogGame, Error, TEXT("Trying to set PawnData [%s] on player state [%s] that already has valid PawnData [%s]."), *GetNameSafe(InPawnData), *GetNameSafe(this), *GetNameSafe(PawnData));
        return;
    }

    MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, PawnData, this);
    PawnData = InPawnData;

    for (const UUR_AbilitySet* AbilitySet : PawnData->AbilitySets)
    {
        if (AbilitySet)
        {
            AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr);
        }
    }

    UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, NAME_GameAbilityReady);

    ForceNetUpdate();
}

void AUR_PlayerState::InternalOnTeamChanged(AUR_PlayerState* PS, int32 OldTeamIndex, int32 NewTeamIndex)
{
    // When local player changes team, update MPC's TeamColor# mappings accordingly
    if (UUR_FunctionLibrary::IsLocallyControlled(PS))
    {
        UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("InternalOnTeamChanged: %i -> %i"), OldTeamIndex, NewTeamIndex));

        const auto Params = GetDefault<UUR_MPC_Global>();
        TArray<FName> ParamNames = { Params->P_TeamColor0, Params->P_TeamColor1, Params->P_TeamColor2, Params->P_TeamColor3 };
        if (NewTeamIndex < 0 || NewTeamIndex > 3)
        {
            // if we have no team or an unhandled team number, try to prioritize the standard,
            // aka. Enemy on team0 and Enemy2 on team1. Ally ends up on team3 which is wrong though.
            FName Last = ParamNames.Pop(EAllowShrinking::No);
            ParamNames.Insert(Last, 0);
        }
        if (NewTeamIndex >= 1)
        {
            FName MyTeamParam = ParamNames[NewTeamIndex];
            ParamNames.RemoveAt(NewTeamIndex);
            ParamNames.Insert(MyTeamParam, 0);
        }
        /*
        UUR_MPC_Global::SetVector(this, ParamNames[0], UUR_MPC_Global::GetVector(this, Params->P_AllyColor));
        UUR_MPC_Global::SetVector(this, ParamNames[1], UUR_MPC_Global::GetVector(this, Params->P_EnemyColor));
        UUR_MPC_Global::SetVector(this, ParamNames[2], UUR_MPC_Global::GetVector(this, Params->P_EnemyColor2));
        UUR_MPC_Global::SetVector(this, ParamNames[3], UUR_MPC_Global::GetVector(this, Params->P_EnemyColor3));
        */
        // Parameters mapping for the win
        UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("MPC: Map %s to AllyColor"), *ParamNames[0].ToString()), false);
        UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("MPC: Map %s to EnemyColor"), *ParamNames[1].ToString()), false);
        UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("MPC: Map %s to EnemyColor2"), *ParamNames[2].ToString()), false);
        UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("MPC: Map %s to EnemyColor3"), *ParamNames[3].ToString()), false);
        UUR_MPC_Global::MapParameter(this, ParamNames[0], Params->P_AllyColor);
        UUR_MPC_Global::MapParameter(this, ParamNames[1], Params->P_EnemyColor);
        UUR_MPC_Global::MapParameter(this, ParamNames[2], Params->P_EnemyColor2);
        UUR_MPC_Global::MapParameter(this, ParamNames[3], Params->P_EnemyColor3);
    }

    if (auto Char = GetPawn<AUR_Character>())
    {
        Char->UpdateTeamColor();
    }
}

FLinearColor AUR_PlayerState::GetColor()
{
    if (IsOnlyASpectator())
    {
        return UUR_FunctionLibrary::GetSpectatorDisplayTextColor();
    }

    const auto Params = GetDefault<UUR_MPC_Global>();
    if (TeamIndex >= 0 && TeamIndex <= 3)
    {
        FName ParamName = TArray<FName>({ Params->P_TeamColor0, Params->P_TeamColor1, Params->P_TeamColor2, Params->P_TeamColor3 })[TeamIndex];
        return UUR_MPC_Global::GetVector(this, ParamName);
    }
    else
    {
        const auto PC = UUR_FunctionLibrary::GetLocalPlayerController(this);
        if (IUR_TeamInterface::Execute_IsAlly(this, (UObject*)PC))
        {
            return UUR_MPC_Global::GetVector(this, Params->P_AllyColor);
        }
    }
    return UUR_MPC_Global::GetVector(this, Params->P_EnemyColor);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

bool AUR_PlayerState::IsAWinner()
{
    if (AUR_GameState* GS = GetWorld()->GetGameState<AUR_GameState>())
    {
        if (GS->Winner)
        {
            if (GS->Winner->IsA(APlayerState::StaticClass()))
            {
                return GS->Winner == this;
            }
            if (GS->Winner->IsA(APawn::StaticClass()))
            {
                return GS->Winner == GetPawn();
            }
            if (GS->Winner->IsA(AUR_TeamInfo::StaticClass()))
            {
                return IUR_TeamInterface::Execute_IsAlly(GS->Winner, this);
            }
        }
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_PlayerState::ServerSetCharacterCustomization_Implementation(const FCharacterCustomization& InCustomization)
{
    //NOTE: We can either do server validation of chosen assets, and correct the assets before replicating them to others
    // Or just replicate directly and let clients decide if they accept those customizations.
    CharacterCustomization = InCustomization;
    OnRep_CharacterCustomization();
}

void AUR_PlayerState::OnRep_CharacterCustomization()
{
    // If customization is replicated after Character, apply it now
    if (auto URChar = GetPawn<AUR_Character>())
    {
        URChar->ApplyCustomization(CharacterCustomization);
    }
}

void AUR_PlayerState::InternalOnPawnSet(APlayerState* PS, APawn* NewPawn, APawn* OldPawn)
{
    // If character is replicated after customization, apply it now
    if (auto URChar = Cast<AUR_Character>(NewPawn))
    {
        URChar->ApplyCustomization(CharacterCustomization);
        //URChar->UpdateTeamColor();    // ApplyCustomization already has to call this because it needs to reset the materials when changing meshes
    }
}

void AUR_PlayerState::OnRep_MyTeamID(FGenericTeamId OldTeamID)
{
    ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}

void AUR_PlayerState::OnRep_MySquadID()
{
    //@TODO: Let the squad subsystem know (once that exists)
}
