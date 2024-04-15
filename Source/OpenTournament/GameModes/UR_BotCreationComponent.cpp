// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_BotCreationComponent.h"
#include "AIController.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

#include "UR_DeveloperSettings.h"
#include "UR_GameMode.h"
#include "GameModes/UR_ExperienceManagerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_BotCreationComponent)

UUR_BotCreationComponent::UUR_BotCreationComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UUR_BotCreationComponent::BeginPlay()
{
    Super::BeginPlay();

    // Listen for the experience load to complete
    AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
    UUR_ExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UUR_ExperienceManagerComponent>();
    check(ExperienceComponent);
    ExperienceComponent->CallOrRegister_OnExperienceLoaded_LowPriority(FOnGameExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
}

void UUR_BotCreationComponent::OnExperienceLoaded(const UUR_ExperienceDefinition* Experience)
{
#if WITH_SERVER_CODE
    if (HasAuthority())
    {
        ServerCreateBots();
    }
#endif
}

#if WITH_SERVER_CODE

void UUR_BotCreationComponent::ServerCreateBots()
{
    if (BotControllerClass == nullptr)
    {
        return;
    }

    RemainingBotNames = RandomBotNames;

    // Determine how many bots to spawn
    int32 EffectiveBotCount = NumBotsToCreate;

    // Give the developer settings a chance to override it
    if (GIsEditor)
    {
        const auto* DeveloperSettings = GetDefault<UUR_DeveloperSettings>();
        if (DeveloperSettings->bOverrideBotCount)
        {
            EffectiveBotCount = DeveloperSettings->OverrideNumPlayerBotsToSpawn;
        }
    }

    // Give the URL a chance to override it
    if (const AGameModeBase* GameModeBase = GetGameMode<AGameModeBase>())
    {
        EffectiveBotCount = UGameplayStatics::GetIntOption(GameModeBase->OptionsString, TEXT("NumBots"), EffectiveBotCount);
    }

    // Create them
    for (int32 Count = 0; Count < EffectiveBotCount; ++Count)
    {
        SpawnOneBot();
    }
}

FString UUR_BotCreationComponent::CreateBotName(int32 PlayerIndex)
{
    FString Result;
    if (RemainingBotNames.Num() > 0)
    {
        const int32 NameIndex = FMath::RandRange(0, RemainingBotNames.Num() - 1);
        Result = RemainingBotNames[NameIndex];
        RemainingBotNames.RemoveAtSwap(NameIndex);
    }
    else
    {
        //@TODO: PlayerId is only being initialized for players right now
        PlayerIndex = FMath::RandRange(260, 260 + 100);
        Result = FString::Printf(TEXT("Tinplate %d"), PlayerIndex);
    }
    return Result;
}

void UUR_BotCreationComponent::SpawnOneBot()
{
    FActorSpawnParameters SpawnInfo;
    SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnInfo.OverrideLevel = GetComponentLevel();
    SpawnInfo.ObjectFlags |= RF_Transient;
    AAIController* NewController = GetWorld()->SpawnActor<AAIController>(BotControllerClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnInfo);

    if (NewController != nullptr)
    {
        AUR_GameMode* GameMode = GetGameMode<AUR_GameMode>();
        check(GameMode);

        if (NewController->PlayerState != nullptr)
        {
            NewController->PlayerState->SetPlayerName(CreateBotName(NewController->PlayerState->GetPlayerId()));
        }

        GameMode->GenericPlayerInitialization(NewController);
        GameMode->RestartPlayer(NewController);

        if (NewController->GetPawn() != nullptr)
        {
            // @! TODO
        }

        SpawnedBotList.Add(NewController);
    }
}

void UUR_BotCreationComponent::RemoveOneBot()
{
    if (SpawnedBotList.Num() > 0)
    {
        // Right now this removes a random bot as they're all the same; could prefer to remove one
        // that's high skill or low skill or etc... depending on why you are removing one
        const int32 BotToRemoveIndex = FMath::RandRange(0, SpawnedBotList.Num() - 1);

        AAIController* BotToRemove = SpawnedBotList[BotToRemoveIndex];
        SpawnedBotList.RemoveAtSwap(BotToRemoveIndex);

        if (BotToRemove)
        {
            // Just destroy the actor // @! TODO HealthComponent behavior
            if (APawn* ControlledPawn = BotToRemove->GetPawn())
            {
                ControlledPawn->Destroy();
            }

            // Destroy the controller (will cause it to Logout, etc...)
            BotToRemove->Destroy();
        }
    }
}

#endif
