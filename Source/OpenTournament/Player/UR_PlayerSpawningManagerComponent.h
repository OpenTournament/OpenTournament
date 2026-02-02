// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Components/GameStateComponent.h>

#include "UR_PlayerSpawningManagerComponent.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AController;
class APlayerController;
class APlayerState;
class APlayerStart;
class AUR_PlayerStart;
class AActor;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @class UUR_PlayerSpawningManagerComponent
 */
UCLASS()
class OPENTOURNAMENT_API UUR_PlayerSpawningManagerComponent : public UGameStateComponent
{
    GENERATED_BODY()

public:
    UUR_PlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer);

    /** UActorComponent */
    virtual void InitializeComponent() override;

    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** ~UActorComponent */

protected:
    // Utility
    APlayerStart* GetFirstRandomUnoccupiedPlayerStart(AController* Controller, const TArray<AUR_PlayerStart*>& FoundStartPoints) const;

    virtual AActor* OnChoosePlayerStart(AController* Player, TArray<AUR_PlayerStart*>& PlayerStarts);

    virtual void OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation);

    UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName=OnFinishRestartPlayer))
    void K2_OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation);

private:
    /** We proxy these calls from AUR_GameMode, to this component so that each experience can more easily customize the respawn system they want. */
#pragma region AUR_GameMode
    AActor* ChoosePlayerStart(AController* Player);
    bool ControllerCanRestart(AController* Player);
    void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation);
    friend class AUR_GameMode;
#pragma endregion AUR_GameMode

    UPROPERTY(Transient)
    TArray<TWeakObjectPtr<AUR_PlayerStart>> CachedPlayerStarts;

private:
    void OnLevelAdded(ULevel* InLevel, UWorld* InWorld);

    void HandleOnActorSpawned(AActor* SpawnedActor);

#if WITH_EDITOR
    APlayerStart* FindPlayFromHereStart(AController* Player);
#endif
};
