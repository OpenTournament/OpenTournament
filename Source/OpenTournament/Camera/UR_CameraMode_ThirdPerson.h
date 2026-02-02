// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UR_CameraMode.h"

#include "Curves/CurveFloat.h"
#include "UR_PenetrationAvoidanceFeeler.h"
#include "DrawDebugHelpers.h"

#include "UR_CameraMode_ThirdPerson.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UCurveVector;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * UUR_CameraMode_ThirdPerson
 *
 *	A basic third person camera mode.
 */
UCLASS(Abstract, Blueprintable)
class UUR_CameraMode_ThirdPerson : public UUR_CameraMode
{
    GENERATED_BODY()

public:
    UUR_CameraMode_ThirdPerson();

protected:
    virtual void UpdateView(float DeltaTime) override;

    void UpdateForTarget(float DeltaTime);
    void UpdatePreventPenetration(float DeltaTime);
    void PreventCameraPenetration(class AActor const& ViewTarget, FVector const& SafeLoc, FVector& CameraLoc, float const& DeltaTime, float& DistBlockedPct, bool bSingleRayOnly);

    virtual void DrawDebug(UCanvas* Canvas) const override;

protected:
    // Curve that defines local-space offsets from the target using the view pitch to evaluate the curve.
    UPROPERTY(EditDefaultsOnly, Category = "Third Person", Meta = (EditCondition = "!bUseRuntimeFloatCurves"))
    TObjectPtr<const UCurveVector> TargetOffsetCurve;

    // UE-103986: Live editing of RuntimeFloatCurves during PIE does not work (unlike curve assets).
    // Once that is resolved this will become the default and TargetOffsetCurve will be removed.
    UPROPERTY(EditDefaultsOnly, Category = "Third Person")
    bool bUseRuntimeFloatCurves;

    UPROPERTY(EditDefaultsOnly, Category = "Third Person", Meta = (EditCondition = "bUseRuntimeFloatCurves"))
    FRuntimeFloatCurve TargetOffsetX;

    UPROPERTY(EditDefaultsOnly, Category = "Third Person", Meta = (EditCondition = "bUseRuntimeFloatCurves"))
    FRuntimeFloatCurve TargetOffsetY;

    UPROPERTY(EditDefaultsOnly, Category = "Third Person", Meta = (EditCondition = "bUseRuntimeFloatCurves"))
    FRuntimeFloatCurve TargetOffsetZ;

    // Alters the speed that a crouch offset is blended in or out
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Third Person")
    float CrouchOffsetBlendMultiplier = 5.0f;

    // Penetration prevention
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision")
    float PenetrationBlendInTime = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision")
    float PenetrationBlendOutTime = 0.15f;

    /** If true, does collision checks to keep the camera out of the world. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision")
    bool bPreventPenetration = true;

    /** If true, try to detect nearby walls and move the camera in anticipation.  Helps prevent popping. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision")
    bool bDoPredictiveAvoidance = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float CollisionPushOutDistance = 2.f;

    /** When the camera's distance is pushed into this percentage of its full distance due to penetration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float ReportPenetrationPercent = 0.f;

    /**
     * These are the feeler rays that are used to find where to place the camera.
     * Index: 0  : This is the normal feeler we use to prevent collisions.
     * Index: 1+ : These feelers are used if you bDoPredictiveAvoidance=true, to scan for potential impacts if the player
     *             were to rotate towards that direction and primitively collide the camera so that it pulls in before
     *             impacting the occluder.
     */
    UPROPERTY(EditDefaultsOnly, Category = "Collision")
    TArray<FUR_PenetrationAvoidanceFeeler> PenetrationAvoidanceFeelers;

    UPROPERTY(Transient)
    float AimLineToDesiredPosBlockedPct;

    UPROPERTY(Transient)
    TArray<TObjectPtr<const AActor>> DebugActorsHitDuringCameraPenetration;

#if ENABLE_DRAW_DEBUG
    mutable float LastDrawDebugTime = -MAX_FLT;
#endif

protected:
    void SetTargetCrouchOffset(FVector NewTargetOffset);
    void UpdateCrouchOffset(float DeltaTime);

    FVector InitialCrouchOffset = FVector::ZeroVector;
    FVector TargetCrouchOffset = FVector::ZeroVector;
    float CrouchOffsetBlendPct = 1.0f;
    FVector CurrentCrouchOffset = FVector::ZeroVector;
};
