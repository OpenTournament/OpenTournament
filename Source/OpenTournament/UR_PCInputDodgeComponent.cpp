// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_PCInputDodgeComponent.h"

#include <EnhancedInputComponent.h>
#include <InputAction.h>

#include "UR_Character.h"
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
        if (auto EnhancedInputComponent = Cast<UEnhancedInputComponent>(OwningPC->InputComponent))
        {
            EnhancedInputComponent->BindAction(InputActionKeyDodge, ETriggerEvent::Triggered, this, &UUR_PCInputDodgeComponent::OnKeyDodge);
            EnhancedInputComponent->BindAction(InputActionTapForward, ETriggerEvent::Triggered, this, &UUR_PCInputDodgeComponent::OnTapForward);
            EnhancedInputComponent->BindAction(InputActionTapBack, ETriggerEvent::Triggered, this, &UUR_PCInputDodgeComponent::OnTapBack);
            EnhancedInputComponent->BindAction(InputActionTapRight, ETriggerEvent::Triggered, this, &UUR_PCInputDodgeComponent::OnTapRight);
            EnhancedInputComponent->BindAction(InputActionTapLeft, ETriggerEvent::Triggered, this, &UUR_PCInputDodgeComponent::OnTapLeft);
            EnhancedInputComponent->BindAction(InputActionTapUp, ETriggerEvent::Triggered, this, &UUR_PCInputDodgeComponent::OnTapUp);
            EnhancedInputComponent->BindAction(InputActionTapDown, ETriggerEvent::Triggered, this, &UUR_PCInputDodgeComponent::OnTapDown);
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
            // Read cached MovementAxes to determine KeyDodge direction. Default to Forward
            if (MovementStrafeAxis > 0.5f)
            {
                Character->SetDodgeDirection(EDodgeDirection::Right);
            }
            else if (MovementStrafeAxis < -0.5f)
            {
                Character->SetDodgeDirection(EDodgeDirection::Left);
            }
            else if (MovementForwardAxis < 0.f)
            {
                Character->SetDodgeDirection(EDodgeDirection::Backward);
            }
            else
            {
                Character->SetDodgeDirection(EDodgeDirection::Forward);
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_PCInputDodgeComponent::OnTapForward(const FInputActionInstance& InputActionInstance)
{
    KeyTapTime.LastTapRightTime = -10.f;
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
