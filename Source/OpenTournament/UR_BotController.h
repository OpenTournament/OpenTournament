// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "AIModule/Classes/AIController.h"
#include "UR_BotController.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Bot player controller class.
 */
UCLASS()
class OPENTOURNAMENT_API AUR_BotController : public AAIController
{
    GENERATED_BODY()

    AUR_BotController();

protected:
    virtual void PostInitializeComponents() override;
    virtual void InitPlayerState() override;
    virtual void OnNewPawnHandler(APawn* P);

public:

    UPROPERTY(BlueprintReadWrite)
    FTimerHandle RespawnTimerHandle;

    // Just a wrapper that calls GameMode->RestartPlayer(), can be used as delegate for SetTimer
    UFUNCTION(BlueprintCallable)
    void Respawn();

    virtual void UpdateControlRotation(float DeltaTime, bool bUpdatePawn) override;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    // Experimental : coding the aim-error directly in the focus, so when we spectate a bot, we can see his real aim.
    // Later down the line we want to delegate "where to aim" to the weapons/firemodes.
    // But the aim error can still be added here as a post-process mechanic.

    // The idea is to regularly generate an point in world space "around" the ideal target point.
    // The error magnitude should increase with relative speed (pseudo on-screen speed) of the target.

    // Ideal aim point, recalculated whenever a new GoalAimPoint is generated (not updated every tick!)
    UPROPERTY(BlueprintReadOnly)
    mutable FVector TrueAimTarget;

    // Current aim point we are interpolating to
    UPROPERTY(BlueprintReadOnly)
    mutable FVector GoalAimPoint;

    // Cache this to reset on change
    mutable const AActor* LastAimActor;

    // Max remaining lifetime of current GoalAimPoint
    mutable float GoalAimPointTime;

    // Current interpolating aim point (ie. current rotation but as a world space point)
    // Its initial distance is set by the distance to TrueAimTarget when a goal is generated
    UPROPERTY(BlueprintReadOnly)
    mutable FVector CurrentAimPoint;

    // Whether to use time-dilated frame time or not.
    UPROPERTY(EditAnywhere, Category = "AimError")
    bool Aim_bUseTimeDilation;
    // Tolerance when checking if we have reached goal
    UPROPERTY(EditAnywhere, Category = "AimError")
    float Aim_GoalTolerance;
    // Max cone angle error when generating goal point from current aim to true target
    UPROPERTY(EditAnywhere, Category = "AimError")
    float Aim_AngleErrorMax;
    // How relative projected speed affects cone angle. ErrorAngle = Lerp(0, Max, Speed/Divisor)
    UPROPERTY(EditAnywhere, Category = "AimError")
    float Aim_AngleSpeedDivisor;
    // How relative projected speed generates distance error along cone.
    UPROPERTY(EditAnywhere, Category = "AimError")
    float Aim_DistanceSpeedFactor;
    // Range of the goal point life time - smaller value forces regenerating points more often
    UPROPERTY(EditAnywhere, Category="AimError")
    FVector2D Aim_PointDuration;
    // Interpolation speed from current aim point (current rot) to GoalAimPoint.
    // Higher value means bot flicks faster, and as a result generates points more often.
    UPROPERTY(EditAnywhere, Category = "AimError")
    float Aim_InterpSpeed;

    // This is where we return our interpolated rotation instead of simply returning Actor->Location (default implementation)
    virtual FVector GetFocalPointOnActor(const AActor* Actor) const override;

    // Calculate a new GoalAimPoint by adding an error going from current rotation towards TrueAimTarget
    virtual FVector CalculateNewGoalAimPoint(const AActor* Actor) const;

    // This is where we should calculate the ideal aim point according to the weapon/firemode we are using, with no error applied.
    virtual FVector CalculateAimTargetForActor(const AActor* Actor) const;
    virtual FVector CalculateAimTargetForLocation(const FVector& WorldLocation) const;
};
