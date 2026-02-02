// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_PlayerSpawningManagerComponent.h"

#include <GameFramework/PlayerState.h>
#include <EngineUtils.h>

#include "UR_PlayerStart.h"

#if WITH_EDITOR
#include <Engine/PlayerStartPIE.h>
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_PlayerSpawningManagerComponent)

/////////////////////////////////////////////////////////////////////////////////////////////////

DEFINE_LOG_CATEGORY_STATIC(LogPlayerSpawning, Log, All);

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_PlayerSpawningManagerComponent::UUR_PlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetIsReplicatedByDefault(false);
    bAutoRegister = true;
    bAutoActivate = true;
    bWantsInitializeComponent = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bAllowTickOnDedicatedServer = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UUR_PlayerSpawningManagerComponent::InitializeComponent()
{
    Super::InitializeComponent();

    FWorldDelegates::LevelAddedToWorld.AddUObject(this, &ThisClass::OnLevelAdded);

    UWorld* World = GetWorld();
    World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateUObject(this, &ThisClass::HandleOnActorSpawned));

    for (TActorIterator<AUR_PlayerStart> It(World); It; ++It)
    {
        if (AUR_PlayerStart* PlayerStart = *It)
        {
            CachedPlayerStarts.Add(PlayerStart);
        }
    }
}

void UUR_PlayerSpawningManagerComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

APlayerStart* UUR_PlayerSpawningManagerComponent::GetFirstRandomUnoccupiedPlayerStart(AController* Controller, const TArray<AUR_PlayerStart*>& StartPoints) const
{
    if (Controller)
    {
        TArray<AUR_PlayerStart*> UnOccupiedStartPoints;
        TArray<AUR_PlayerStart*> OccupiedStartPoints;

        for (AUR_PlayerStart* StartPoint : StartPoints)
        {
            EGamePlayerStartLocationOccupancy State = StartPoint->GetLocationOccupancy(Controller);

            switch (State)
            {
                case EGamePlayerStartLocationOccupancy::Empty:
                {
                    UnOccupiedStartPoints.Add(StartPoint);
                    break;
                }
                case EGamePlayerStartLocationOccupancy::Partial:
                {
                    OccupiedStartPoints.Add(StartPoint);
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        if (UnOccupiedStartPoints.Num() > 0)
        {
            return UnOccupiedStartPoints[FMath::RandRange(0, UnOccupiedStartPoints.Num() - 1)];
        }

        if (OccupiedStartPoints.Num() > 0)
        {
            return OccupiedStartPoints[FMath::RandRange(0, OccupiedStartPoints.Num() - 1)];
        }
    }

    return nullptr;
}

AActor* UUR_PlayerSpawningManagerComponent::OnChoosePlayerStart(AController* Player, TArray<AUR_PlayerStart*>& PlayerStarts)
{
    return nullptr;
}

void UUR_PlayerSpawningManagerComponent::OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation)
{
    // Noop
}

//======================================================================
// AUR_GameMode Proxied Calls - Need to handle when someone chooses
// to restart a player the normal way in the engine.
//======================================================================

AActor* UUR_PlayerSpawningManagerComponent::ChoosePlayerStart(AController* Player)
{
    if (Player)
    {
#if WITH_EDITOR
        if (APlayerStart* PlayerStart = FindPlayFromHereStart(Player))
        {
            return PlayerStart;
        }
#endif

        TArray<AUR_PlayerStart*> StarterPoints;
        for (auto StartIt = CachedPlayerStarts.CreateIterator(); StartIt; ++StartIt)
        {
            if (AUR_PlayerStart* Start = StartIt->Get())
            {
                StarterPoints.Add(Start);
            }
            else
            {
                StartIt.RemoveCurrent();
            }
        }

        if (APlayerState* PlayerState = Player->GetPlayerState<APlayerState>())
        {
            // start dedicated spectators at any random starting location, but they do not claim it
            if (PlayerState->IsOnlyASpectator())
            {
                if (!StarterPoints.IsEmpty())
                {
                    return StarterPoints[FMath::RandRange(0, StarterPoints.Num() - 1)];
                }

                return nullptr;
            }
        }

        AActor* PlayerStart = OnChoosePlayerStart(Player, StarterPoints);

        if (!PlayerStart)
        {
            PlayerStart = GetFirstRandomUnoccupiedPlayerStart(Player, StarterPoints);
        }

        if (AUR_PlayerStart* GameStart = Cast<AUR_PlayerStart>(PlayerStart))
        {
            GameStart->TryClaim(Player);
        }

        return PlayerStart;
    }

    return nullptr;
}

bool UUR_PlayerSpawningManagerComponent::ControllerCanRestart(AController* Player)
{
    bool bCanRestart = true;

    // TODO Can they restart?

    return bCanRestart;
}

void UUR_PlayerSpawningManagerComponent::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
    OnFinishRestartPlayer(NewPlayer, StartRotation);
    K2_OnFinishRestartPlayer(NewPlayer, StartRotation);
}

//================================================================


void UUR_PlayerSpawningManagerComponent::OnLevelAdded(ULevel* InLevel, UWorld* InWorld)
{
    if (InWorld == GetWorld())
    {
        for (AActor* Actor : InLevel->Actors)
        {
            if (AUR_PlayerStart* PlayerStart = Cast<AUR_PlayerStart>(Actor))
            {
                ensure(!CachedPlayerStarts.Contains(PlayerStart));
                CachedPlayerStarts.Add(PlayerStart);
            }
        }
    }
}

void UUR_PlayerSpawningManagerComponent::HandleOnActorSpawned(AActor* SpawnedActor)
{
    if (AUR_PlayerStart* PlayerStart = Cast<AUR_PlayerStart>(SpawnedActor))
    {
        CachedPlayerStarts.Add(PlayerStart);
    }
}

#if WITH_EDITOR
APlayerStart* UUR_PlayerSpawningManagerComponent::FindPlayFromHereStart(AController* Player)
{
    // Only 'Play From Here' for a player controller, bots etc. should all spawn from normal spawn points.
    if (Player->IsA<APlayerController>())
    {
        if (UWorld* World = GetWorld())
        {
            for (TActorIterator<APlayerStart> It(World); It; ++It)
            {
                if (APlayerStart* PlayerStart = *It)
                {
                    if (PlayerStart->IsA<APlayerStartPIE>())
                    {
                        // Always prefer the first "Play from Here" PlayerStart, if we find one while in PIE mode
                        return PlayerStart;
                    }
                }
            }
        }
    }

    return nullptr;
}
#endif //WITH_EDITOR
