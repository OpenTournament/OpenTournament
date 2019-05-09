// Copyright 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_CharacterMovementComponent.h"

#include "Engine/World.h"
#include "Components/CapsuleComponent.h"

#include "OpenTournament.h"
#include "UR_Character.h"
#include "UR_PlayerController.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_CharacterMovementComponent::UUR_CharacterMovementComponent(const class FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , bIsDodging(false)
    , DodgeResetTime(0.0f)
    , DodgeResetInterval(0.35f)
    , DodgeImpulseHorizontal(1500.f)
    , DodgeImpulseVertical(525.f)
    , DodgeLandingSpeedScale(0.1f)
{
    // Unreal Movement Settings
    // GroundSpeed
    // DodgeImpulseHorizontal(1500.f)
    // DodgeImpulseVertical(400.f)
    // DodgeLandingSpeedScale(0.1f)

    // Unreal Tournament Movement Settings
    // GroundSpeed = 1200
    // AirSpeed = 1200
    // AccelRate = 2048 (? units)
    // AirControl 0.35

}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_CharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
    const auto URCharacterOwner = Cast<AUR_Character>(CharacterOwner);
    const bool bIsClient = (GetNetMode() == NM_Client && CharacterOwner->Role == ROLE_AutonomousProxy);

    UMovementComponent::TickComponent(DeltaTime, TickType, ThisTickFunction);

    const FVector InputVector = ConsumeInputVector();
    if (!HasValidData() || ShouldSkipUpdate(DeltaTime) || UpdatedComponent->IsSimulatingPhysics())
    {
        return;
    }

    if (URCharacterOwner->Role > ROLE_SimulatedProxy)
    {
        if (URCharacterOwner->Role == ROLE_Authority)
        {
            // Check we are still in the world, and stop simulating if not.
            const bool bStillInWorld = (bCheatFlying || CharacterOwner->CheckStillInWorld());
            if (!bStillInWorld || !HasValidData())
            {
                return;
            }
        }

        // Allow root motion to move characters that have no controller.
        if (CharacterOwner->IsLocallyControlled() || bRunPhysicsWithNoController || (!CharacterOwner->Controller && CharacterOwner->IsPlayingRootMotion()))
        {
            FNetworkPredictionData_Client_Character* ClientData = ((CharacterOwner->Role < ROLE_Authority) && (GetNetMode() == NM_Client)) ? GetPredictionData_Client_Character() : nullptr;
            if (ClientData)
            {
                // Update our delta time for physics simulation.
                DeltaTime = UpdateTimeStampAndDeltaTime(DeltaTime, ClientData);
                CurrentServerMoveTime = ClientData->CurrentTimeStamp;
            }
            else
            {
                AdjustMovementTimers(1.f*DeltaTime);
                CurrentServerMoveTime = GetWorld()->GetTimeSeconds();
            }

            // We need to check the jump state before adjusting input acceleration, to minimize latency
            // and to make sure acceleration respects our potentially new falling state.
            CharacterOwner->CheckJumpInput(DeltaTime);

            // apply input to acceleration
            Acceleration = ScaleInputAcceleration(ConstrainInputAcceleration(InputVector));
            AnalogInputModifier = ComputeAnalogInputModifier();

            if ((CharacterOwner->Role == ROLE_Authority))
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
        }
    }
}

void UUR_CharacterMovementComponent::ProcessLanded(const FHitResult & Hit, float remainingTime, int32 Iterations)
{
    if (GetOwner())
    {
        if (bIsDodging)
        {
            Velocity *= DodgeLandingSpeedScale;
            DodgeResetTime = GetWorld()->TimeSeconds + DodgeResetInterval; // Get Server adjusted Time
        }

        Super::ProcessLanded(Hit, remainingTime, Iterations);

        bIsDodging = false;
    }
}

float UUR_CharacterMovementComponent::UpdateTimeStampAndDeltaTime(float DeltaTime, FNetworkPredictionData_Client_Character * ClientData)
{
    const float UnModifiedTimeStamp = ClientData->CurrentTimeStamp + DeltaTime;
    DeltaTime = ClientData->UpdateTimeStampAndDeltaTime(DeltaTime, *CharacterOwner, *this);
    if (ClientData->CurrentTimeStamp < UnModifiedTimeStamp)
    {
        // client timestamp rolled over, so roll over our movement timers
        AdjustMovementTimers(UnModifiedTimeStamp - ClientData->CurrentTimeStamp);
    }
    return DeltaTime;
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
    if (CharacterOwner && CharacterOwner->bPressedJump)
    {
        if ((MovementMode == MOVE_Walking) || (MovementMode == MOVE_Falling))
        {
            DoJump(CharacterOwner->bClientUpdating);
        }
        else if ((MovementMode == MOVE_Swimming) && CanDodge())
        {
            // @! TODO edge of water jump
        }
    }
    else if (DodgeDirection != EDodgeDirection::DD_None)
    {
        AUR_Character* URCharacterOwner = Cast<AUR_Character>(CharacterOwner);
        if (URCharacterOwner)
        {
            // Standard Dodges
            if (!(DodgeDirection == EDodgeDirection::DD_Up || DodgeDirection == EDodgeDirection::DD_Down))
            {
                const FRotator TurnRot(0.f, CharacterOwner->GetActorRotation().Yaw, 0.f);
                const FRotationMatrix TurnRotMatrix = FRotationMatrix(TurnRot);

                const float DodgeDirX = (DodgeDirection == EDodgeDirection::DD_Forward)
                    ? 1.f
                    : (DodgeDirection == EDodgeDirection::DD_Backward ? -1.f : 0.f);
                const float DodgeDirY = (DodgeDirection == EDodgeDirection::DD_Left)
                    ? -1.f
                    : (DodgeDirection == EDodgeDirection::DD_Right ? 1.f : 0.f);
                const float DodgeCrossX = (DodgeDirY == 1.f || DodgeDirY == -1.f) ? 1.f : 0.f;
                const float DodgeCrossY = (DodgeDirX == 1.f || DodgeDirX == -1.f) ? 1.f : 0.f;

                const FVector XAxis = TurnRotMatrix.GetScaledAxis(EAxis::X);
                const FVector YAxis = TurnRotMatrix.GetScaledAxis(EAxis::Y);

                URCharacterOwner->Dodge((DodgeDirX * XAxis + DodgeDirY * YAxis).GetSafeNormal(),
                    (DodgeCrossX * XAxis + DodgeCrossY * YAxis).GetSafeNormal());
            }
            // Swim Dodges
            else if (Is3DMovementMode() && (DodgeDirection != EDodgeDirection::DD_Up || DodgeDirection == EDodgeDirection::DD_Down))
            {
                const FRotator TurnRot(0.f, CharacterOwner->GetActorRotation().Yaw, 0.f);
                const FRotationMatrix TurnRotMatrix = FRotationMatrix(TurnRot);

                const float DodgeDirX = 0.f;
                const float DodgeDirZ = (DodgeDirection == EDodgeDirection::DD_Up)
                    ? 1.f
                    : (DodgeDirection == EDodgeDirection::DD_Down ? -1.f : 0.f);

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

bool UUR_CharacterMovementComponent::CanDodge()
{
    return CanEverJump() && (DodgeResetTime <= 0.f);
}

bool UUR_CharacterMovementComponent::PerformDodge(FVector &DodgeDir, FVector &DodgeCross)
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
        // @! TODO Functionalize to : PerformDodgeImpulse()
        Velocity = DodgeImpulseHorizontal*DodgeDir + (Velocity | DodgeCross)*DodgeCross;
        Velocity.Z = 0.f;
        float SpeedXY = FMath::Min(Velocity.Size(), DodgeImpulseHorizontal); //

        Velocity = SpeedXY*Velocity.GetSafeNormal();
        Velocity.Z = DodgeImpulseVertical;

        bIsDodging = true;
        bNotifyApex = true;
        SetMovementMode(MOVE_Falling);

        URCharacterOwner->OnDodge(URCharacterOwner->GetActorLocation(), Velocity);
    }
    else if (IsFalling())
    {
        if (CurrentWallDodgeCount < MaxWallDodges)
        {
            // @! TODO Handle wall dodges
        }
        
    }
    else if (IsSwimming())
    {
        Velocity = DodgeImpulseHorizontal*DodgeDir + (Velocity | DodgeCross)*DodgeCross;
        float SpeedXY = FMath::Min(Velocity.Size(), DodgeImpulseHorizontal); //

        bIsDodging = true;
        bNotifyApex = true;

        URCharacterOwner->OnDodge(URCharacterOwner->GetActorLocation(), Velocity);
        // @! TODO Swim Dodge needs its own DodgeReset time value or have a timer that resets a dodge
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

void UUR_CharacterMovementComponent::ClearDodgeInput()
{
    DodgeDirection = EDodgeDirection::DD_None;
}