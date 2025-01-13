// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_PCInputDodgeComponent.h"

#include <InputAction.h>

#include "UR_Character.h"
#include "UR_InputComponent.h"
#include "UR_PlayerController.h"
#include "Character/UR_CharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_PCInputDodgeComponent)

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_PCInputDodgeComponent::UUR_PCInputDodgeComponent()
    : MovementForwardAxis(0.0f)
    , MovementStrafeAxis(0.0f)
    , bRequestedKeyDodge(false)
    , MaxDodgeClickThresholdTime(0.25f)
    , KeyTapTime(FKeyTapTime())
{
    KeyTapTime.LastTapLeftTime = -10.f;
    KeyTapTime.LastTapRightTime = -10.f;
    KeyTapTime.LastTapForwardTime = -10.f;
    KeyTapTime.LastTapBackTime = -10.f;
    KeyTapTime.LastTapUpTime = -10.f;
    KeyTapTime.LastTapDownTime = -10.f;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_PCInputDodgeComponent::BeginPlay()
{
    Super::BeginPlay();

    if (const auto OwningPC = Cast<AUR_PlayerController>(GetOwner()))
    {
        if (auto InputComponent = Cast<UUR_InputComponent>(OwningPC->InputComponent))
        {
            //InputComponent->BindAction(InputActionKeyDodge, ETriggerEvent::Triggered, this, &ThisClass::OnKeyDodge);

            //InputComponent->BindAction(InputActionTapForward, ETriggerEvent::Triggered, this, &ThisClass::OnTapForward);
            //InputComponent->BindAction(InputActionTapBack, ETriggerEvent::Triggered, this, &ThisClass::OnTapBack);
            //InputComponent->BindAction(InputActionTapRight, ETriggerEvent::Triggered, this, &ThisClass::OnTapRight);
            //InputComponent->BindAction(InputActionTapLeft, ETriggerEvent::Triggered, this, &ThisClass::OnTapLeft);
            //InputComponent->BindAction(InputActionTapUp, ETriggerEvent::Triggered, this, &ThisClass::OnTapUp);
            //InputComponent->BindAction(InputActionTapDown, ETriggerEvent::Triggered, this, &ThisClass::OnTapDown);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_PCInputDodgeComponent::ProcessPlayerInput(const float DeltaTime, const bool bGamePaused)
{
    if (bRequestedKeyDodge)
    {
        SetKeyDodgeInputDirection();
    }

    bRequestedKeyDodge = false;
    MovementForwardAxis = 0.f;
    MovementStrafeAxis = 0.f;
}

void UUR_PCInputDodgeComponent::OnKeyDodge(const FInputActionInstance& InputActionInstance)
{
    bRequestedKeyDodge = true;
}

void UUR_PCInputDodgeComponent::SetKeyDodgeInputDirection() const
{
    if (const auto OwningPC = Cast<AUR_PlayerController>(GetOwner()))
    {
        if (const auto Character = Cast<AUR_Character>(OwningPC->GetCharacter()))
        {
            EDodgeDirection DodgeDirection = EDodgeDirection::Forward;

            auto LastMovementInput = Character->GetLastMovementInputVector();
            if (LastMovementInput.IsNearlyZero())
            {
                Character->SetDodgeDirection(DodgeDirection);
                return;
            }

            const FVector ForwardVector = Character->GetActorForwardVector();
            const FVector RightVector = FVector::CrossProduct(FVector::UpVector, ForwardVector).GetSafeNormal();

            // Use the dot product to determine direction
            const float ForwardDot = FVector::DotProduct(LastMovementInput, ForwardVector);
            const float RightDot = FVector::DotProduct(LastMovementInput, RightVector);

            if (ForwardDot > 0.707f)
            {
                DodgeDirection = EDodgeDirection::Forward;
            }
            else if (ForwardDot < - 0.707f)
            {
                DodgeDirection = EDodgeDirection::Backward;
            }
            else if (RightDot > 0.707f)
            {
                DodgeDirection = EDodgeDirection::Right;
            }
            else if (RightDot < -0.707f)
            {
                DodgeDirection = EDodgeDirection::Left;
            }

            Character->SetDodgeDirection(DodgeDirection);

            // Read cached MovementAxes to determine KeyDodge direction. Default to Forward
            // if (MovementStrafeAxis > 0.5f)
            // {
            //     Character->SetDodgeDirection(EDodgeDirection::Right);
            // }
            // else if (MovementStrafeAxis < -0.5f)
            // {
            //     Character->SetDodgeDirection(EDodgeDirection::Left);
            // }
            // else if (MovementForwardAxis < 0.f)
            // {
            //     Character->SetDodgeDirection(EDodgeDirection::Backward);
            // }
            // else
            // {
            //     Character->SetDodgeDirection(EDodgeDirection::Forward);
            // }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_PCInputDodgeComponent::OnTapForward(const FInputActionInstance& InputActionInstance)
{
    KeyTapTime.LastTapRightTime = INDEX_NONE;
    KeyTapTime.LastTapLeftTime = -10.f;
    KeyTapTime.LastTapBackTime = -10.f;
    KeyTapTime.LastTapUpTime = -10.f;
    KeyTapTime.LastTapDownTime = -10.f;

    SetTapDodgeInputDirection(KeyTapTime.LastTapForwardTime, MaxDodgeClickThresholdTime, EDodgeDirection::Forward);
    KeyTapTime.LastTapForwardTime = GetWorld()->GetTimeSeconds();
}

void UUR_PCInputDodgeComponent::OnTapBack(const FInputActionInstance& InputActionInstance)
{
    KeyTapTime.LastTapRightTime = -10.f;
    KeyTapTime.LastTapLeftTime = -10.f;
    KeyTapTime.LastTapForwardTime = -10.f;
    KeyTapTime.LastTapUpTime = -10.f;
    KeyTapTime.LastTapDownTime = -10.f;

    SetTapDodgeInputDirection(KeyTapTime.LastTapBackTime, MaxDodgeClickThresholdTime, EDodgeDirection::Backward);
    KeyTapTime.LastTapBackTime = GetWorld()->GetTimeSeconds();
}

void UUR_PCInputDodgeComponent::OnTapLeft(const FInputActionInstance& InputActionInstance)
{
    KeyTapTime.LastTapRightTime = -10.f;
    KeyTapTime.LastTapForwardTime = -10.f;
    KeyTapTime.LastTapBackTime = -10.f;
    KeyTapTime.LastTapUpTime = -10.f;
    KeyTapTime.LastTapDownTime = -10.f;

    SetTapDodgeInputDirection(KeyTapTime.LastTapLeftTime, MaxDodgeClickThresholdTime, EDodgeDirection::Left);
    KeyTapTime.LastTapLeftTime = GetWorld()->GetTimeSeconds();
}

void UUR_PCInputDodgeComponent::OnTapRight(const FInputActionInstance& InputActionInstance)
{
    KeyTapTime.LastTapLeftTime = -10.f;
    KeyTapTime.LastTapForwardTime = -10.f;
    KeyTapTime.LastTapBackTime = -10.f;
    KeyTapTime.LastTapUpTime = -10.f;
    KeyTapTime.LastTapDownTime = -10.f;

    SetTapDodgeInputDirection(KeyTapTime.LastTapRightTime, MaxDodgeClickThresholdTime, EDodgeDirection::Right);
    KeyTapTime.LastTapRightTime = GetWorld()->GetTimeSeconds();
}

void UUR_PCInputDodgeComponent::OnTapUp(const FInputActionInstance& InputActionInstance)
{
    KeyTapTime.LastTapRightTime = -10.f;
    KeyTapTime.LastTapLeftTime = -10.f;
    KeyTapTime.LastTapForwardTime = -10.f;
    KeyTapTime.LastTapBackTime = -10.f;
    KeyTapTime.LastTapDownTime = -10.f;

    SetTapDodgeInputDirection(KeyTapTime.LastTapUpTime, MaxDodgeClickThresholdTime, EDodgeDirection::Up);
    KeyTapTime.LastTapUpTime = GetWorld()->GetTimeSeconds();
}

void UUR_PCInputDodgeComponent::OnTapDown(const FInputActionInstance& InputActionInstance)
{
    KeyTapTime.LastTapRightTime = -10.f;
    KeyTapTime.LastTapLeftTime = -10.f;
    KeyTapTime.LastTapForwardTime = -10.f;
    KeyTapTime.LastTapBackTime = -10.f;
    KeyTapTime.LastTapUpTime = -10.f;

    SetTapDodgeInputDirection(KeyTapTime.LastTapDownTime, MaxDodgeClickThresholdTime, EDodgeDirection::Down);
    KeyTapTime.LastTapDownTime = GetWorld()->GetTimeSeconds();
}

void UUR_PCInputDodgeComponent::SetTapDodgeInputDirection(const float LastTapTime, const float MaxClickTime,
                                                          const EDodgeDirection DodgeDirection) const
{
    if (const auto OwningPC = Cast<AUR_PlayerController>(GetOwner()))
    {
        if (const auto Character = Cast<AUR_Character>(OwningPC->GetCharacter()))
        {
            if (GetWorld()->GetTimeSeconds() - LastTapTime < MaxClickTime)
            {
                Character->SetDodgeDirection(DodgeDirection);
            }
        }
    }
}
