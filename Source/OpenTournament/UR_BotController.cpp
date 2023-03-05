// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_BotController.h"

#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameModeBase.h"


/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_BotController::AUR_BotController()
{
    bWantsPlayerState = true;

    Aim_bUseTimeDilation = true;    //useful for debugging in slomo
    Aim_GoalTolerance = 10.f;
    Aim_AngleErrorMax = 45.f;
    Aim_AngleSpeedDivisor = 1000.f;
    Aim_DistanceSpeedFactor = 0.3f;
    Aim_PointDuration = FVector2D(0.2f, 0.4f);
    Aim_InterpSpeed = 10.f;
}

void AUR_BotController::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    OnNewPawn.AddUObject(this, &AUR_BotController::OnNewPawnHandler);
}

void AUR_BotController::InitPlayerState()
{
    Super::InitPlayerState();

    if (!IsNetMode(NM_Client))
    {
        if (auto PS = GetPlayerState<APlayerState>())
        {
            PS->SetPlayerNameInternal(FString::Printf(TEXT("OTBot-%d"), FMath::RandRange(10, 9999)));
        }
    }
}

void AUR_BotController::OnNewPawnHandler(APawn* P)
{
    if (P)
    {
        GetWorld()->GetTimerManager().ClearTimer(RespawnTimerHandle);
    }
    else
    {
        //TODO: Obey gamemode's respawn rules
        float Delay = FMath::RandRange(1.f, 4.f);
        //NOTE: We use loop to attempt respawn every second in case respawn fails
        GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, FTimerDelegate::CreateUObject(this, &AUR_BotController::Respawn), 1.f, true, Delay);
    }
}

void AUR_BotController::Respawn()
{
    if (auto GM = GetWorld()->GetAuthGameMode())
    {
        GM->RestartPlayer(this);
    }
}

void AUR_BotController::UpdateControlRotation(float DeltaTime, bool bUpdatePawn)
{
    Super::UpdateControlRotation(DeltaTime, bUpdatePawn);

    if (bUpdatePawn)
    {
        if (auto P = GetPawn())
            P->SetRemoteViewPitch(GetControlRotation().Pitch);  //FIXME: doesn't work for spectating
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Aim Experiments
/////////////////////////////////////////////////////////////////////////////////////////////////

FVector AUR_BotController::GetFocalPointOnActor(const AActor* Actor) const
{
    if (!Actor)
        return FAISystem::InvalidLocation;

    float dt = Aim_bUseTimeDilation ? GetWorld()->GetDeltaSeconds() : GetWorld()->DeltaRealTimeSeconds;

    GoalAimPointTime -= dt;

    // If our target changed, or if we reached our aim point, or if this is taking too long
    if (Actor != LastAimActor || CurrentAimPoint.Equals(GoalAimPoint, Aim_GoalTolerance) || GoalAimPointTime < 0.f)
    {
        TrueAimTarget = CalculateAimTargetForActor(Actor);
        GoalAimPoint = CalculateNewGoalAimPoint(Actor);
        GoalAimPointTime = FMath::FRandRange(Aim_PointDuration.X, Aim_PointDuration.Y);
    }

    LastAimActor = Actor;

    CurrentAimPoint = FMath::VInterpTo(CurrentAimPoint, GoalAimPoint, dt, Aim_InterpSpeed);

    return CurrentAimPoint;
}

FVector AUR_BotController::CalculateNewGoalAimPoint(const AActor* Actor) const
{
    const FVector& MyLoc = GetPawn()->GetPawnViewLocation();
    const FRotator& MyRot = GetControlRotation();

    const FVector& ScreenPlaneNormal = (TrueAimTarget - MyLoc).GetSafeNormal();

    const FVector& RelativeVel = Actor->GetVelocity() - GetPawn()->GetVelocity();
    const FVector& OnScreenVel = FVector::VectorPlaneProject(RelativeVel, ScreenPlaneNormal);
    const float OnScreenSpeed = OnScreenVel.Size();

    CurrentAimPoint = MyLoc + MyRot.Vector() * (TrueAimTarget - MyLoc).Size();

    // Define a cone, starting from the point we are looking at, towards TrueTarget
    const FVector& ConeAxis = (TrueAimTarget - CurrentAimPoint).GetSafeNormal();
    const float Distance = (TrueAimTarget - CurrentAimPoint).Size();

    const float ConeAngle = FMath::Clamp(OnScreenSpeed / Aim_AngleSpeedDivisor, 0.f, Aim_AngleErrorMax);
    const float DistanceError = OnScreenSpeed * Aim_DistanceSpeedFactor;
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

FVector AUR_BotController::CalculateAimTargetForActor(const AActor* Actor) const
{
    //TODO: if projectile weapon, take velocity/accel of target into account

    return CalculateAimTargetForLocation(Actor->GetActorLocation());
}

FVector AUR_BotController::CalculateAimTargetForLocation(const FVector& WorldLocation) const
{
    //TODO: if lobbing weapon, calculate Z toss

    return WorldLocation;
}
