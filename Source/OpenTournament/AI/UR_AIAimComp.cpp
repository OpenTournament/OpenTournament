// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_AIAimComp.h"

#include <GameFramework/Pawn.h>

#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "AIHelpers.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_AIAimComp::UUR_AIAimComp()
{
    SetAutoActivate(true);

    bUseTimeDilation = true;
    GoalTolerance = 10.f;
    AngleErrorMax = 45.f;
    AngleSpeedDivisor = 1000.f;
    DistanceSpeedFactor = 0.3f;
    PointDuration = FVector2D(0.2f, 0.4f);
    InterpSpeed = 10.f;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Aim Experiments
/////////////////////////////////////////////////////////////////////////////////////////////////

FVector UUR_AIAimComp::ApplyAimCorrectionForTargetActor(const AController* MyController, const AActor* TargetActor)
{
    if (!TargetActor || !MyController || !MyController->GetPawn())
        return FAISystem::InvalidLocation;

    float dt = bUseTimeDilation ? GetWorld()->GetDeltaSeconds() : GetWorld()->DeltaRealTimeSeconds;

    GoalAimPointTime -= dt;

    // If our target changed, or if we reached our aim point, or if this is taking too long
    if (TargetActor != LastAimActor || CurrentAimPoint.Equals(GoalAimPoint, GoalTolerance) || GoalAimPointTime < 0.f)
    {
        TrueAimTarget = CalculateAimTargetForActor(TargetActor);
        GoalAimPoint = CalculateNewGoalAimPoint(MyController, TargetActor);
        GoalAimPointTime = FMath::FRandRange(PointDuration.X, PointDuration.Y);
    }

    LastAimActor = TargetActor;

    CurrentAimPoint = FMath::VInterpTo(CurrentAimPoint, GoalAimPoint, dt, InterpSpeed);

    return CurrentAimPoint;
}

FVector UUR_AIAimComp::CalculateNewGoalAimPoint(const AController* MyController, const AActor* TargetActor)
{
    const FVector& MyLoc = MyController->GetPawn()->GetPawnViewLocation();
    const FRotator& MyRot = GetOwner<AController>()->GetControlRotation();

    const FVector& ScreenPlaneNormal = (TrueAimTarget - MyLoc).GetSafeNormal();

    const FVector& RelativeVel = TargetActor->GetVelocity() - MyController->GetPawn()->GetVelocity();
    const FVector& OnScreenVel = FVector::VectorPlaneProject(RelativeVel, ScreenPlaneNormal);
    const float OnScreenSpeed = OnScreenVel.Size();

    CurrentAimPoint = MyLoc + MyRot.Vector() * (TrueAimTarget - MyLoc).Size();

    // Define a cone, starting from the point we are looking at, towards TrueTarget
    const FVector& ConeAxis = (TrueAimTarget - CurrentAimPoint).GetSafeNormal();
    const float Distance = (TrueAimTarget - CurrentAimPoint).Size();

    const float ConeAngle = FMath::Clamp(OnScreenSpeed / AngleSpeedDivisor, 0.f, AngleErrorMax);
    const float DistanceError = OnScreenSpeed * DistanceSpeedFactor;
    const float DistanceMin = FMath::Max(0, Distance - DistanceError);
    const float DistanceMax = DistanceMin + 2 * DistanceError;

    FVector Result = CurrentAimPoint + FMath::FRandRange(DistanceMin, DistanceMax) * FMath::VRandCone(ConeAxis, ConeAngle);

    // Point shouldn't end up BEHIND me (if enemy is close and radius gets large)
    if (FVector::DotProduct(GoalAimPoint - MyLoc, TrueAimTarget - MyLoc) < 0.f)
    {
        // Mirror by me
        Result += (Result - MyLoc).Size2D() * 2.f * (TrueAimTarget - MyLoc).GetSafeNormal2D();
    }

    return Result;
}

FVector UUR_AIAimComp::CalculateAimTargetForActor(const AActor* Actor)
{
    //TODO: if projectile weapon, take velocity/accel of target into account
    const FVector& TargetLoc = Actor->GetActorLocation();

    return CalculateAimTargetForLocation(TargetLoc);
}

FVector UUR_AIAimComp::CalculateAimTargetForLocation(const FVector& WorldLocation)
{
    //TODO: if lobbing weapon, calculate Z toss

    return WorldLocation;
}
