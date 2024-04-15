// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <ModularAIController.h>

#include "UR_BotController.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Bot player controller class.
 */
UCLASS()
class OPENTOURNAMENT_API AUR_BotController
    : public AModularAIController
{
    GENERATED_BODY()

    AUR_BotController();

protected:
    virtual void InitPlayerState() override;

    virtual void OnNewPawnHandler(APawn* P);

    virtual void UpdateControlRotation(float DeltaTime, bool bUpdatePawn) override;

    virtual FVector GetFocalPointOnActor(const AActor* Actor) const override;

public:
    UPROPERTY(VisibleAnywhere)
    class UUR_AIAimComp* AimComponent;

    UPROPERTY(VisibleAnywhere)
    class UUR_AINavigationJumpingComp* NavigationJumpingComponent;

    UPROPERTY(BlueprintReadWrite)
    FTimerHandle RespawnTimerHandle;

    // Just a wrapper that calls GameMode->RestartPlayer(), can be used as delegate for SetTimer
    UFUNCTION(BlueprintCallable)
    void Respawn();
};
