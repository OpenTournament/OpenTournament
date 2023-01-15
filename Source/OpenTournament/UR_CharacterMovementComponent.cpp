// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_CharacterMovementComponent.h"

#include "Engine/World.h"
#include "Components/CapsuleComponent.h"

#include "OpenTournament.h"
#include "Enums/UR_MovementAction.h"
#include "Interfaces/UR_WallDodgeSurfaceInterface.h"
#include "UR_Character.h"
#include "UR_PlayerController.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_CharacterMovementComponent::UUR_CharacterMovementComponent(const class FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer),
    bIsDodging(false),
    DodgeResetTime(0.0f),
    DodgeResetInterval(0.35f),
    DodgeImpulseHorizontal(1500.f),
    DodgeImpulseVertical(525.f),
    DodgeLandingSpeedScale(0.1f),
    bCanSlopeBoost(true),
    SlopeBoostAssistVelocityZThreshold(8.f),
    SlopeBoostScale(1.15f),
    SlopeImpactNormalZ(0.2f),
    SlopeSlideRadiusScale(1.0f),
    bCanWallDodge(true),
    WallDodgeBehavior(EWallDodgeBehavior::DisallowSurface),
    WallDodgeTraceDistance(54.5f),
    WallDodgeMinimumNormal(0.5f),
    WallDodgeImpulseHorizontal(1500.f),
    WallDodgeImpulseVertical(525.f),
    WallDodgeResetInterval(0.2f),
    bCanBoostDodge(false),
    WallDodgeVelocityZPreservationThreshold(-10000.f),
    WallDodgeFallingVelocityCancellationThreshold(0.f),
    CurrentWallDodgeCount(0),
    MaxWallDodges(1)
{
    MaxWalkSpeed = 1000.f;
    MaxWalkSpeedCrouched = 400.f;
    MaxSwimSpeed = 600.f;
    JumpZVelocity = 812.5f;
    GroundFriction = 8.f;
    BrakingDecelerationWalking = 5120.f;
    AirControl = 0.35f;
    MaxAcceleration = 5120.f;
    MaxStepHeight = 62.5f;

    SetCrouchedHalfHeight(53.625);

    NavAgentProps.bCanCrouch = true;
    NavAgentProps.bCanSwim = true;
    NavAgentProps.bCanJump = true;
    NavAgentProps.bCanWalk = true;

    bUseFlatBaseForFloorChecks = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_CharacterMovementComponent::SetupMovementProperties()
{
    // SWITCH
}

void UUR_CharacterMovementComponent::SetupMovementPropertiesGeneration0()
{
    // Unreal Movement Settings
    GroundFriction = 8.f;
    BrakingDecelerationWalking = 5120.f;
    AirControl = 0.35f;
    MaxAcceleration = 5120.f;
    MaxStepHeight = 62.5f;

    MaxWalkSpeed = 1000.f;
    MaxWalkSpeedCrouched = 400.f;
    MaxSwimSpeed = 500.f;
    JumpZVelocity = 812.5f;

    DodgeResetInterval = 0.35f;
    DodgeImpulseHorizontal = 1500.f;
    DodgeImpulseVertical = 400.f;
    DodgeLandingSpeedScale = 0.1f;

    bCanSlopeBoost = true;
    SlopeBoostAssistVelocityZThreshold = 8.f;
    SlopeBoostScale = 1.15f;
    SlopeImpactNormalZ = 0.2f;
    SlopeSlideRadiusScale = 1.0f;

    bCanWallDodge = true;
    WallDodgeBehavior = EWallDodgeBehavior::RequiresSurface;
    WallDodgeTraceDistance = 54.5f;
    WallDodgeMinimumNormal = 0.5f;
    WallDodgeImpulseHorizontal = 1500.f;
    WallDodgeImpulseVertical = 400.f;
    WallDodgeResetInterval = 0.35f;
    bCanBoostDodge = false;
    WallDodgeVelocityZPreservationThreshold = -10000.f;
    MaxWallDodges = 1;
}

void UUR_CharacterMovementComponent::SetupMovementPropertiesGeneration1()
{
    // UT99 Movement Settings
    GroundFriction = 8.f;
    BrakingDecelerationWalking = 5120.f;
    AirControl = 0.35f;
    MaxAcceleration = 5120.f;
    MaxStepHeight = 62.5f;

    MaxWalkSpeed = 1000.f;
    MaxWalkSpeedCrouched = 400.f;
    MaxSwimSpeed = 500.f;
    JumpZVelocity = 812.5f;

    DodgeResetInterval = 0.35f;
    DodgeImpulseHorizontal = 1500.f;
    DodgeImpulseVertical = 525.f;
    DodgeLandingSpeedScale = 0.1f;

    bCanSlopeBoost = true;
    SlopeBoostAssistVelocityZThreshold = 8.f;
    SlopeBoostScale = 1.15f;
    SlopeImpactNormalZ = 0.2f;
    SlopeSlideRadiusScale = 1.0f;

    bCanWallDodge = true;
    WallDodgeBehavior = EWallDodgeBehavior::RequiresSurface;
    WallDodgeTraceDistance = 54.5f;
    WallDodgeMinimumNormal = 0.5f;
    WallDodgeImpulseHorizontal = 1500.f;
    WallDodgeImpulseVertical = 525.f;
    WallDodgeResetInterval = 0.35f;
    bCanBoostDodge = false;
    WallDodgeVelocityZPreservationThreshold = -10000.f;
    MaxWallDodges = 1;
}

void UUR_CharacterMovementComponent::SetupMovementPropertiesGeneration2()
{
    // UT2003 Movement Properties
    GroundFriction = 8.f;
    BrakingDecelerationWalking = 5120.f;
    AirControl = 0.35f;
    MaxAcceleration = 5120.f;
    MaxStepHeight = 87.5f;

    MaxWalkSpeed = 1100.f;
    MaxWalkSpeedCrouched = 440.f;
    MaxSwimSpeed = 550.f;
    JumpZVelocity = 850.f;

    bCanWallDodge = true;
    WallDodgeBehavior = EWallDodgeBehavior::DisallowSurface;
    WallDodgeTraceDistance = 80.f; // Based on 32 unit trace from Radius to Wall, scaled by 2.5x
    WallDodgeMinimumNormal = 0.5f;
    WallDodgeImpulseHorizontal = 1500.f;
    WallDodgeImpulseVertical = 525.f;
    WallDodgeResetInterval = 0.35f;
    bCanBoostDodge = true;
    WallDodgeVelocityZPreservationThreshold = -250.f;
    MaxWallDodges = 1;
}

void UUR_CharacterMovementComponent::SetupMovementPropertiesGeneration2_Scaled()
{
    // TODO
}

void UUR_CharacterMovementComponent::SetupMovementPropertiesGeneration3()
{
    // UT2004 Movement Properties
    GroundFriction = 8.f;
    BrakingDecelerationWalking = 5120.f;
    AirControl = 0.35f;
    MaxAcceleration = 5120.f;
    MaxStepHeight = 87.5f;

    MaxWalkSpeed = 1100.f;
    MaxWalkSpeedCrouched = 440.f;
    MaxSwimSpeed = 550.f;
    JumpZVelocity = 850.f;

    bCanWallDodge = true;
    WallDodgeBehavior = EWallDodgeBehavior::DisallowSurface;
    WallDodgeTraceDistance = 80.f; // Based on 32 unit trace from Radius to Wall, scaled by 2.5x
    WallDodgeMinimumNormal = 0.5f;
    WallDodgeImpulseHorizontal = 1500.f;
    WallDodgeImpulseVertical = 525.f;
    WallDodgeResetInterval = 0.35f;
    bCanBoostDodge = false;
    WallDodgeVelocityZPreservationThreshold = -10000.f;
    MaxWallDodges = 1;
}

void UUR_CharacterMovementComponent::SetupMovementPropertiesGeneration3_Scaled()
{
    // TODO
}

void UUR_CharacterMovementComponent::SetupMovementPropertiesGeneration4()
{
    // UT3

    bCanBoostDodge = false;
    WallDodgeVelocityZPreservationThreshold = -500.f;
}

void UUR_CharacterMovementComponent::SetupMovementPropertiesGeneration4_Scaled()
{
    // TODO
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_CharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    const auto URCharacterOwner = Cast<AUR_Character>(CharacterOwner);
    const bool bIsClient = (GetNetMode() == NM_Client && CharacterOwner->GetLocalRole() == ROLE_AutonomousProxy);

    UMovementComponent::TickComponent(DeltaTime, TickType, ThisTickFunction);

    const FVector InputVector = ConsumeInputVector();
    if (!HasValidData() || ShouldSkipUpdate(DeltaTime) || UpdatedComponent->IsSimulatingPhysics())
    {
        return;
    }

    if (URCharacterOwner->GetLocalRole() > ROLE_SimulatedProxy)
    {
        if (URCharacterOwner->GetLocalRole() == ROLE_Authority)
        {
            // Check we are still in the world, and stop simulating if not.
            const bool bStillInWorld = (bCheatFlying || CharacterOwner->CheckStillInWorld());
            if (!bStillInWorld || !HasValidData())
            {
                return;
            }
        }

        // If we are a client we might have received an update from the server.
        if (bIsClient)
        {
            ClientUpdatePositionAfterServerUpdate();
        }

        // Allow root motion to move characters that have no controller.
        if (CharacterOwner->IsLocallyControlled() || bRunPhysicsWithNoController || (!CharacterOwner->Controller && CharacterOwner->IsPlayingRootMotion()))
        {
            FNetworkPredictionData_Client_Character* ClientData = ((CharacterOwner->GetLocalRole() < ROLE_Authority) && (GetNetMode() == NM_Client)) ? GetPredictionData_Client_Character() : nullptr;
            if (ClientData)
            {
                CurrentServerMoveTime = ClientData->CurrentTimeStamp;
            }
            else
            {
                AdjustMovementTimers(1.f * DeltaTime);
                CurrentServerMoveTime = GetWorld()->GetTimeSeconds();
            }

            // We need to check the jump state before adjusting input acceleration, to minimize latency
            // and to make sure acceleration respects our potentially new falling state.
            CharacterOwner->CheckJumpInput(DeltaTime);

            // apply input to acceleration
            Acceleration = ScaleInputAcceleration(ConstrainInputAcceleration(InputVector));
            AnalogInputModifier = ComputeAnalogInputModifier();

            if ((CharacterOwner->GetLocalRole() == ROLE_Authority))
            {
                PerformMovement(DeltaTime);
            }
            else if (bIsClient)
            {
                ReplicateMoveToServer(DeltaTime, Acceleration);
            }
        }
        else if ((CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy))
        {
            // Server ticking for remote client.
            // Between net updates from the client we need to update position if based on another object,
            // otherwise the object will move on intermediate frames and we won't follow it.
            MaybeUpdateBasedMovement(DeltaTime);
            SaveBaseLocation();

            // Smooth on listen server for local view of remote clients. We may receive updates at a rate different than our own tick rate.
            if (!bNetworkSmoothingComplete && IsNetMode(NM_ListenServer))
            {
                SmoothClientPosition(DeltaTime);
            }
        }
    }
    else if (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)
    {
        AdjustProxyCapsuleSize();
        SimulatedTick(DeltaTime);
        CharacterOwner->RecalculateBaseEyeHeight();
    }
}

void UUR_CharacterMovementComponent::ProcessLanded(const FHitResult& Hit, float RemainingTime, int32 Iterations)
{
    Super::ProcessLanded(Hit, RemainingTime, Iterations);

    if (auto Owner = Cast<AUR_Character>(GetOwner()))
    {
        Owner->UnCrouch(false);

        if (bIsDodging)
        {
            Velocity *= DodgeLandingSpeedScale;
            DodgeResetTime = GetWorld()->TimeSeconds + DodgeResetInterval; // Get Server adjusted Time
            bIsDodging = false;
            Owner->UpdateGameplayTags(FGameplayTagContainer{ Owner->GetMovementActionGameplayTag(EMovementAction::Dodging) }, FGameplayTagContainer{});
        }

        if (bIsJumping)
        {
            bIsJumping = false;
            Owner->UpdateGameplayTags(FGameplayTagContainer{ Owner->GetMovementActionGameplayTag(EMovementAction::Jumping) }, FGameplayTagContainer{});
        }
    }

    CurrentWallDodgeCount = 0;
}

FVector UUR_CharacterMovementComponent::ComputeSlideVector(const FVector& Delta, const float Time, const FVector& Normal, const FHitResult& Hit) const
{
    FVector Result = Super::ComputeSlideVector(Delta, Time, Normal, Hit);

    // prevent boosting up slopes
    //if (IsFalling())
    //{
    //    Result = HandleSlopeBoosting(Result, Delta, Time, Normal, Hit);
    //}

    return Result;
}

FVector UUR_CharacterMovementComponent::HandleSlopeBoosting(const FVector& SlideResult, const FVector& Delta, const float Time, const FVector& Normal, const FHitResult& Hit) const
{
    FVector Result = SlideResult;

    // prevent boosting up slopes
    if (Result.Z > 0.f)
    {
        float PawnRadius = -1.f;
        float PawnHalfHeight = -1.f;
        CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);
        if (Delta.Z < 0.f && (Hit.ImpactNormal.Z < 0.08f)) // MAX_STEP_SIDE_Z = 0.08
        {
            // We were moving downward, but a slide was going to send us upward. We want to aim
            // straight down for the next move to make sure we get the most upward-facing opposing normal.
            Result = FVector(0.f, 0.f, Delta.Z);
        }
        else if (bCanSlopeBoost && (((CharacterOwner->GetActorLocation() - Hit.ImpactPoint).Size2D() > SlopeSlideRadiusScale * PawnRadius) || (Hit.ImpactNormal.Z > SlopeImpactNormalZ)))
        {
            if (Result.Z > Delta.Z*Time)
            {
                if (Result.Z > 0.f)
                {
                    // @! TODO : This is a first pass. Re-evaluate later
                    const float MaxVelocityZ = FMath::Max(SlopeBoostAssistVelocityZThreshold, Result.Z);
                    const float ScaledVelocityZ = 1.f - ((MaxVelocityZ - Result.Z) / MaxVelocityZ);
                    const float ClampedVelocityZ = FMath::Clamp(ScaledVelocityZ, 0.f, 1.f);
                    const float SlopeScale = FMath::Lerp(1.f, SlopeBoostScale, ClampedVelocityZ);

                    Result.Z = FMath::Max(Result.Z * SlopeScale, Delta.Z*Time);
                }
                else
                {
                    Result.Z = FMath::Max(Result.Z * 1.f, Delta.Z*Time);
                }
            }
        }
    }
    return Result;
}

void UUR_CharacterMovementComponent::AdjustMovementTimers(float Adjustment)
{
    DodgeResetTime -= Adjustment;
}


bool UUR_CharacterMovementComponent::CanJump()
{
    // @! TODO DoubleJump/DodgeJump
    return (IsMovingOnGround() && CanEverJump() && !bWantsToCrouch);
}


void UUR_CharacterMovementComponent::CheckJumpInput(float DeltaTime)
{
    AUR_Character* URCharacterOwner = Cast<AUR_Character>(CharacterOwner);
    if (URCharacterOwner)
    {
        const EDodgeDirection DodgeDirection{ URCharacterOwner->DodgeDirection };
        if (CharacterOwner->bPressedJump)
        {
            if ((MovementMode == MOVE_Walking) || (MovementMode == MOVE_Falling))
            {
                if (DoJump(CharacterOwner->bClientUpdating))
                {
                    bIsJumping = true;
                    URCharacterOwner->UpdateGameplayTags(FGameplayTagContainer{}, FGameplayTagContainer{ URCharacterOwner->GetMovementActionGameplayTag(EMovementAction::Jumping) });
                }

                // If we didn't perform a jump, reset the bPressedJump flag to prevent OnJump effects
                CharacterOwner->bPressedJump = bIsJumping;
            }
            else if ((MovementMode == MOVE_Swimming) && CanDodge())
            {
                // @! TODO edge of water jump
            }
        }
        else if (DodgeDirection != EDodgeDirection::None)
        {
            // Standard Dodges
            if (!(DodgeDirection == EDodgeDirection::Up || DodgeDirection == EDodgeDirection::Down))
            {
                const FRotator TurnRot(0.f, CharacterOwner->GetActorRotation().Yaw, 0.f);
                const FRotationMatrix TurnRotMatrix = FRotationMatrix(TurnRot);

                const float DodgeDirX = (DodgeDirection == EDodgeDirection::Forward)
                    ? 1.f
                    : (DodgeDirection == EDodgeDirection::Backward ? -1.f : 0.f);
                const float DodgeDirY = (DodgeDirection == EDodgeDirection::Left)
                    ? -1.f
                    : (DodgeDirection == EDodgeDirection::Right ? 1.f : 0.f);
                const float DodgeCrossX = (DodgeDirY == 1.f || DodgeDirY == -1.f) ? 1.f : 0.f;
                const float DodgeCrossY = (DodgeDirX == 1.f || DodgeDirX == -1.f) ? 1.f : 0.f;

                const FVector XAxis = TurnRotMatrix.GetScaledAxis(EAxis::X);
                const FVector YAxis = TurnRotMatrix.GetScaledAxis(EAxis::Y);

                URCharacterOwner->Dodge((DodgeDirX * XAxis + DodgeDirY * YAxis).GetSafeNormal(),
                    (DodgeCrossX * XAxis + DodgeCrossY * YAxis).GetSafeNormal());
            }
            // Swim Dodges
            else if (Is3DMovementMode() && (DodgeDirection != EDodgeDirection::Up || DodgeDirection == EDodgeDirection::Down))
            {
                const FRotator TurnRot(0.f, CharacterOwner->GetActorRotation().Yaw, 0.f);
                const FRotationMatrix TurnRotMatrix = FRotationMatrix(TurnRot);

                const float DodgeDirX = 0.f;
                const float DodgeDirZ = (DodgeDirection == EDodgeDirection::Up)
                    ? 1.f
                    : (DodgeDirection == EDodgeDirection::Down ? -1.f : 0.f);

                const float DodgeCrossX = (DodgeDirZ == 1.f || DodgeDirZ == -1.f) ? 1.f : 0.f;
                const float DodgeCrossZ = 0.f;

                const FVector XAxis = TurnRotMatrix.GetScaledAxis(EAxis::X);
                const FVector ZAxis = TurnRotMatrix.GetScaledAxis(EAxis::Z);

                URCharacterOwner->Dodge((DodgeDirX * XAxis + DodgeDirZ * ZAxis).GetSafeNormal(),
                    (DodgeCrossX * XAxis + DodgeCrossZ * ZAxis).GetSafeNormal());
            }
        }
    }
}

bool UUR_CharacterMovementComponent::DoJump(bool bReplayingMoves)
{
    if (CharacterOwner && CharacterOwner->CanJump())
    {
        const auto bIsFallingAndMultiJumpCapable = IsFalling() ? CharacterOwner->JumpMaxCount > 1 : Super::DoJump(bReplayingMoves);
        if (bIsFallingAndMultiJumpCapable)
        {
            return true;
        }
    }
    return false;
}

bool UUR_CharacterMovementComponent::CanDodge() const
{
    return CanEverJump() && (DodgeResetTime <= 0.f);
}

bool UUR_CharacterMovementComponent::PerformDodge(FVector& DodgeDir, FVector& DodgeCross)
{
    if (!HasValidData())
    {
        return false;
    }

    AUR_Character* URCharacterOwner = Cast<AUR_Character>(CharacterOwner);

    if (!URCharacterOwner)
    {
        return false;
    }

    float PreviousVelocityZ = Velocity.Z;

    if (IsMovingOnGround())
    {
        PerformDodgeImpulse(DodgeDir, DodgeCross);

        bIsDodging = true;
        bNotifyApex = true;
        SetMovementMode(MOVE_Falling);

        URCharacterOwner->OnDodge(URCharacterOwner->GetActorLocation(), Velocity);
    }
    else if (IsFalling())
    {
        if (bCanWallDodge && CurrentWallDodgeCount < MaxWallDodges)
        {
            FHitResult HitResult;
            const bool bHitWallDodgeSurface = TraceWallDodgeSurface(DodgeDir, HitResult);

            if (!bHitWallDodgeSurface)
            {
                return false;
            }

            // We found a WallDodge permitting surface, now a valid WallDodge angle?
            SetWallDodgeDirection(DodgeDir, DodgeCross, HitResult);

            DodgeResetTime = GetWorld()->TimeSeconds + WallDodgeResetInterval;
            CurrentWallDodgeCount++;

            // Trigger Falling Damage, if any
            URCharacterOwner->Landed(HitResult);

            PerformWallDodgeImpulse(DodgeDir, DodgeCross);

            URCharacterOwner->OnWallDodge(URCharacterOwner->GetActorLocation(), Velocity);
        }
    }
    else if (IsSwimming())
    {
        Velocity = DodgeImpulseHorizontal * DodgeDir + (Velocity | DodgeCross) * DodgeCross;
        float SpeedXY = FMath::Min(Velocity.Size(), DodgeImpulseHorizontal); //

        bIsDodging = true;
        bNotifyApex = true;

        URCharacterOwner->OnDodge(URCharacterOwner->GetActorLocation(), Velocity);
        // @! TODO Swim Dodge needs its own DodgeReset time value or have a timer that resets a dodge
    }

    if (bIsDodging)
    {
        URCharacterOwner->UpdateGameplayTags(FGameplayTagContainer{}, FGameplayTagContainer{ URCharacterOwner->GetMovementActionGameplayTag(EMovementAction::Dodging) });
    }

    if (!IsMovingOnGround())
    {
        if (IsFalling())
        {
            return false;
        }
        // @! TODO Swimming, Flying, etc
    }

    return true;
}

void UUR_CharacterMovementComponent::PerformDodgeImpulse(const FVector& DodgeDir, const FVector& DodgeCross)
{
    Velocity = DodgeImpulseHorizontal * DodgeDir + (Velocity | DodgeCross) * DodgeCross;
    Velocity.Z = 0.f;
    const float SpeedXY = FMath::Min(Velocity.Size(), DodgeImpulseHorizontal); //

    Velocity = SpeedXY * Velocity.GetSafeNormal();
    Velocity.Z = DodgeImpulseVertical;
}

void UUR_CharacterMovementComponent::PerformWallDodgeImpulse(FVector& DodgeDir, FVector& DodgeCross)
{
    Velocity = WallDodgeImpulseHorizontal * DodgeDir + (Velocity | DodgeCross) * DodgeCross;
    Velocity.Z = 0.f;
    const float SpeedXY = FMath::Min(Velocity.Size(), WallDodgeImpulseHorizontal);

    Velocity = SpeedXY * Velocity.GetSafeNormal();
    Velocity.Z = GetWallDodgeVerticalImpulse();
}

float UUR_CharacterMovementComponent::GetWallDodgeVerticalImpulse() const
{
    const float CurrentVelocityZ{ static_cast<float>(Velocity.Z) };
    float CachedVelocityZ = 0.f;

    if (CurrentVelocityZ < -1.f * WallDodgeImpulseVertical * 0.5f)
    {
        CachedVelocityZ += CurrentVelocityZ + (WallDodgeImpulseVertical * 0.5f);
    }

    if (bCanBoostDodge || CurrentVelocityZ < WallDodgeVelocityZPreservationThreshold)
    {
        CachedVelocityZ = CurrentVelocityZ + WallDodgeImpulseVertical;
    }
    else
    {
        CachedVelocityZ = WallDodgeImpulseVertical;
    }

    return CachedVelocityZ;
}

bool UUR_CharacterMovementComponent::TraceWallDodgeSurface(const FVector& DodgeDir, OUT FHitResult& HitResult) const
{
    FVector TraceEnd = -1.f * DodgeDir;
    float PawnCapsuleRadius = 0;
    float PawnCapsuleHalfHeight = 0;
    CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleSize(PawnCapsuleRadius, PawnCapsuleHalfHeight);

    const float TraceBoxSize = FMath::Min(0.25f * PawnCapsuleHalfHeight, 0.7f * PawnCapsuleRadius);
    FVector TraceStart = CharacterOwner->GetActorLocation();
    TraceStart.Z -= 0.5f * TraceBoxSize;
    TraceEnd = TraceStart - (WallDodgeTraceDistance + PawnCapsuleRadius - 0.5f * TraceBoxSize) * DodgeDir;

    static const FName DodgeTag = FName(TEXT("Dodge"));
    const FCollisionQueryParams QueryParams(DodgeTag, false, CharacterOwner);

    bool bIsWallDodgeSurfaceHit = GetWorld()->SweepSingleByChannel(HitResult, TraceStart, TraceEnd, FQuat::Identity, UpdatedComponent->GetCollisionObjectType(), FCollisionShape::MakeSphere(TraceBoxSize), QueryParams);

    // Is our HitActor using the WallDodgeSurface interface?
    // We use "ImplementsInterface" method because it may be implemented in Blueprint
    if (HitResult.GetActor() != nullptr && HitResult.GetActor()->GetClass()->ImplementsInterface(UUR_WallDodgeSurfaceInterface::StaticClass()))
    {
        // Invoke the interface function to determine if WallDodging is permitted
        if (!IUR_WallDodgeSurfaceInterface::Execute_IsWallDodgePermitted(HitResult.GetActor()))
        {
            // @! TODO Event Hook for effects?
            bIsWallDodgeSurfaceHit = false;
        }

        // @! TODO Allow modification of WallDodge by the surface (distance, dodge count, etc)
    }
    else if (WallDodgeBehavior == EWallDodgeBehavior::RequiresSurface)
    {
        bIsWallDodgeSurfaceHit = false;
    }

    return bIsWallDodgeSurfaceHit;
}

void UUR_CharacterMovementComponent::SetWallDodgeDirection(OUT FVector& DodgeDir, OUT FVector& DodgeCross, const FHitResult& HitResult) const
{
    if ((HitResult.ImpactNormal | DodgeDir) < WallDodgeMinimumNormal)
    {
        // clamp dodge direction based on wall normal
        FVector ForwardDir = (HitResult.ImpactNormal ^ FVector(0.f, 0.f, 1.f)).GetSafeNormal();
        if ((ForwardDir | DodgeDir) < 0.f)
        {
            ForwardDir *= -1.f;
        }
        DodgeDir = HitResult.ImpactNormal * WallDodgeMinimumNormal * WallDodgeMinimumNormal + ForwardDir * (1.f - WallDodgeMinimumNormal * WallDodgeMinimumNormal);
        DodgeDir = DodgeDir.GetSafeNormal();
        const FVector NewDodgeCross = (DodgeDir ^ FVector(0.f, 0.f, 1.f)).GetSafeNormal();
        DodgeCross = ((NewDodgeCross | DodgeCross) < 0.f) ? -1.f * NewDodgeCross : NewDodgeCross;
    }
}

void UUR_CharacterMovementComponent::ClearDodgeInput()
{
    AUR_Character* URCharacterOwner = Cast<AUR_Character>(CharacterOwner);
    if (URCharacterOwner)
    {
        URCharacterOwner->DodgeDirection = EDodgeDirection::None;
    }
}
