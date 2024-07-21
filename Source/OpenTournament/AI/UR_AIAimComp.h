// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Components/ActorComponent.h"

#include "UR_AIAimComp.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AController;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Aim component for AI controllers.
 */
UCLASS(HideCategories = (Sockets, Tags, ComponentTick, ComponentReplication, Cooking, AssetUserData, Replication, Collision))
class OPENTOURNAMENT_API UUR_AIAimComp
    : public UActorComponent
{
    GENERATED_BODY()

    UUR_AIAimComp();

public:

    // Experimental : coding the aim-error directly in the focus, so when we spectate a bot, we can see his real aim.
    // Later down the line we want to delegate "where to aim" to the weapons/firemodes.
    // But the aim error can still be added here as a post-process mechanic.

    // The idea is to regularly generate an point in world space "around" the ideal target point.
    // The error magnitude should increase with relative speed (pseudo on-screen speed) of the target.

    // Ideal aim point, recalculated whenever a new GoalAimPoint is generated (not updated every tick!)
    UPROPERTY(BlueprintReadOnly)
    FVector TrueAimTarget;

    // Current aim point we are interpolating to
    UPROPERTY(BlueprintReadOnly)
    FVector GoalAimPoint;

    // Cache this to detect changes
    const AActor* LastAimActor;

    // Max remaining lifetime of current GoalAimPoint
    float GoalAimPointTime;

    // Current interpolating aim point (ie. current rotation but as a world space point)
    // Its initial distance is set by the distance to TrueAimTarget when a goal is generated
    UPROPERTY(BlueprintReadOnly)
    FVector CurrentAimPoint;

    // Whether to use time-dilated frame time or not.
    UPROPERTY(EditAnywhere, Category = "AimError")
    bool bUseTimeDilation;
    // Tolerance when checking if we have reached goal
    UPROPERTY(EditAnywhere, Category = "AimError")
    float GoalTolerance;
    // Max cone angle error when generating goal point from current aim to true target
    UPROPERTY(EditAnywhere, Category = "AimError")
    float AngleErrorMax;
    // How relative projected speed affects cone angle. ErrorAngle = Lerp(0, Max, Speed/Divisor)
    UPROPERTY(EditAnywhere, Category = "AimError")
    float AngleSpeedDivisor;
    // How relative projected speed generates distance error along cone.
    UPROPERTY(EditAnywhere, Category = "AimError")
    float DistanceSpeedFactor;
    // Range of the goal point life time - smaller value forces regenerating points more often
    UPROPERTY(EditAnywhere, Category="AimError")
    FVector2D PointDuration;
    // Interpolation speed from current aim point (current rot) to GoalAimPoint.
    // Higher value means bot flicks faster, and as a result generates points more often.
    UPROPERTY(EditAnywhere, Category = "AimError")
    float InterpSpeed;

    // Entry point called by Controller->GetFocalPointOnActor
    // Return value is a point in the world that the controller should aim at.
    // We return our interpolated rotation instead of simply returning Actor->Location (default implementation)
    // This is meant to be called once per frame
    virtual FVector ApplyAimCorrectionForTargetActor(const AController* MyController, const AActor* TargetActor);

protected:

    // Calculate a new GoalAimPoint by adding an error going from current rotation towards TrueAimTarget
    virtual FVector CalculateNewGoalAimPoint(const AController* MyController, const AActor* Actor);

    // This is where we should calculate the ideal aim point according to the weapon/firemode we are using, with no error applied.
    virtual FVector CalculateAimTargetForActor(const AActor* Actor);
    virtual FVector CalculateAimTargetForLocation(const FVector& WorldLocation);
};
