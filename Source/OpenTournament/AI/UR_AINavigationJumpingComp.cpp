// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_AINavigationJumpingComp.h"

#include <KismetTraceUtils.h>

#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include <KismetTraceUtils.h>

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_AINavigationJumpingComp::UUR_AINavigationJumpingComp()
{
    SetAutoActivate(true);
    PrimaryComponentTick.bCanEverTick = true;

    JumpCheckInterval = 0.250f;
    CapsuleRadiusMult = 0.90f;
    MinGroundOffsetForTrace = 10.f;
    AboveHeadOffset = 40.f;
    MaxStepupAngleDot = 0.1f;
    //TraceDistance = 400.f;
    DebugHitDuration = 2.f;
}

void UUR_AINavigationJumpingComp::OnRegister()
{
    Super::OnRegister();

    AIController = GetOwner<AAIController>();
    if (AIController)
    {
        AIController->GetOnNewPawnNotifier().AddUObject(this, &UUR_AINavigationJumpingComp::SetPawn);
    }
}

void UUR_AINavigationJumpingComp::SetPawn(APawn* NewPawn)
{
    MyChar = Cast<ACharacter>(NewPawn);
    CharMoveComp = MyChar ? MyChar->GetCharacterMovement() : nullptr;
    if (CharMoveComp)
    {
        GetWorld()->GetTimerManager().SetTimer(CheckJumpTimerHandle, this, &UUR_AINavigationJumpingComp::CheckJump, JumpCheckInterval, true);
        SetComponentTickEnabled(true);
    }
    else
    {
        GetWorld()->GetTimerManager().ClearTimer(CheckJumpTimerHandle);
        SetComponentTickEnabled(false);
    }
}

// By default AIController releases all movement while in falling state.
// We want to always push towards destination during jumps (or even falls).
// NOTE: This can be generalized to Pawn/PawnMoveComp, but only Character can Jump() so we don't really bother.
void UUR_AINavigationJumpingComp::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (IsActive() && CharMoveComp && CharMoveComp->IsFalling())
    {
        const FVector& Dest = AIController->GetImmediateMoveDestination();
        if (!Dest.IsZero())
        {
            const FVector& Direction2D = (Dest - CharMoveComp->GetActorLocation()).GetSafeNormal2D();
            CharMoveComp->AddInputVector(Direction2D);
        }
    }
}

void UUR_AINavigationJumpingComp::CheckJump()
{
    if (!IsActive() || !CharMoveComp || !CharMoveComp->IsWalking())
        return;

    const FVector& Dest = AIController->GetImmediateMoveDestination();
    if (Dest.IsZero())
        return;

    // We do a capsule trace forward such that :
    // - capsule bottom should be slightly offset to not immediately hit ground when going uphill
    // - capsule top should match player head or even a bit above to ensure we can jump
    // - the bottom spherical part of the capsule should match character's MaxStepHeight. If we hit bottom part we can stepup so no need to jump
    // - radius should ideally be character radius, but this interferes with the above condition

    // If StepHeight < CapsuleRadius, then it doesn't matter if we use CapsuleRadius we can still detect if bottom hits are below or above StepHeight
    // If StepHeight > CapsuleRadius, we will use CapsuleRadius anyways and offset the capsule upwards so that the flat part of the capsule begins at StepHeight

    const float Radius = CapsuleRadiusMult * CharMoveComp->UpdatedComponent->Bounds.BoxExtent.X;
    const float BottomOffset = FMath::Max(MinGroundOffsetForTrace, CharMoveComp->MaxStepHeight - Radius);
    const float Height = (2 * CharMoveComp->UpdatedComponent->Bounds.BoxExtent.Z) + AboveHeadOffset - BottomOffset;

    // NOTE: This calculates distance to peak jump height - not sure if it's better to do it this way or configure a manual distance
    // NOTE: Adding 10% seems to give better results
    const float TraceDistance = 1.1f * FMath::Abs(CharMoveComp->MaxWalkSpeed * (CharMoveComp->JumpZVelocity / CharMoveComp->GetGravityZ()));

    const FCollisionShape Capsule = FCollisionShape::MakeCapsule(Radius, Height / 2.f);
    const FVector& TraceStart = CharMoveComp->GetActorFeetLocation() + FVector(0, 0, BottomOffset + Capsule.GetCapsuleHalfHeight());
    const FVector& Direction2D = (Dest - CharMoveComp->GetActorLocation()).GetSafeNormal2D();

    const FVector& TraceEnd = TraceStart + TraceDistance * Direction2D;

    FCollisionQueryParams Params("AINavigationJumping_TraceCheck", SCENE_QUERY_STAT_ONLY(AINavigationTraces), false, MyChar);
    Params.bIgnoreTouches = true;

    FHitResult Hit;
    //NOTE: Not sure about CollisionChannel, using WorldStatic for now to ensure we can land onto the thing
    GetWorld()->SweepSingleByChannel(Hit, TraceStart, TraceEnd, FQuat::Identity, ECollisionChannel::ECC_WorldStatic, Capsule, Params);

#if ENABLE_DRAW_DEBUG
    if (bDebugTraces)
    {
        DrawDebugCapsuleTraceSingle(GetWorld(), TraceStart, TraceEnd, Capsule.GetCapsuleRadius(), Capsule.GetCapsuleHalfHeight(), EDrawDebugTrace::ForDuration, Hit.bBlockingHit, Hit, FColor::Blue, FColor::Cyan, Hit.bBlockingHit ? DebugHitDuration : JumpCheckInterval);
    }
#endif

    if (!Hit.bBlockingHit)
    {
        return;
    }

    // If blocking hit is further away than our destination, we don't need to jump
    //TODO: Would be better to also check the direction of the *next* destination
    if (FVector::DistSquaredXY(CharMoveComp->GetActorLocation(), Hit.Location) > FVector::DistSquaredXY(CharMoveComp->GetActorLocation(), Dest) + 2 * Radius * Radius)
    {
        return;
    }

    // Check if the hit happened below StepHeight
    const float HitHeight = Hit.ImpactPoint.Z - CharMoveComp->GetActorFeetLocation().Z;
    //NOTE: If we upgrade CollisionChannel, maybe we can check stepup of the object here
    if (HitHeight < CharMoveComp->MaxStepHeight)
    {
        // Ramp check - stepup doesn't work on angled steps. Also, ramps might hit capsule on the bottom but continue much higher
        // ImpactNormal should return the normal that is most-opposing to the trace, so if we hit the angle of a step it should work fine for us
        const float Dot = Hit.ImpactNormal.Dot(FVector::ZAxisVector);

        // Check if we can stepup (step must be almost vertical)
        if (Dot < MaxStepupAngleDot)
        {
            if (bDebugTraces)
                DrawDebugLine(GetWorld(), CharMoveComp->GetActorFeetLocation(), Hit.ImpactPoint, FColor::Yellow, false, DebugHitDuration, SDPG_World, 3.f);
            return;
        }

        // Check if surface is walkable (going uphill)
        if (Dot > FMath::Cos(FMath::DegreesToRadians(CharMoveComp->GetWalkableFloorAngle())))
        {
            if (bDebugTraces)
                DrawDebugLine(GetWorld(), Hit.ImpactPoint, Hit.ImpactPoint + 200 * Hit.ImpactNormal, FColor::Purple, false, DebugHitDuration, SDPG_World, 3.f);
            return;
        }

        // Stepup and walking won't work
    }

    if (bDebugTraces)
        DrawDebugLine(GetWorld(), Hit.ImpactPoint, Hit.ImpactPoint + 200 * Hit.ImpactNormal, FColor::Magenta, false, DebugHitDuration, SDPG_World, 3.f);

    // We need to jump!
    MyChar->Jump();
}
