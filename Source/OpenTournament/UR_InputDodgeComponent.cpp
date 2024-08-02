// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_InputDodgeComponent.h"

#include <EnhancedInputComponent.h>
#include <InputAction.h>
#include "UR_Character.h"
#include "UR_InputComponent.h"
#include "Character/UR_CharacterMovementComponent.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_InputDodgeComponent::UUR_InputDodgeComponent()
    : MaxDodgeClickThresholdTime(0.25f)
    , LastKeyTap(FKeyTapTime())
{
}

void UUR_InputDodgeComponent::SetupInputComponent(UUR_InputComponent* InputComponent)
{
    InputComponent->BindAction(InputActionKeyDodge, ETriggerEvent::Triggered, this, &UUR_InputDodgeComponent::OnKeyDodge);
    InputComponent->BindAction(InputActionMove, ETriggerEvent::Triggered, this, &UUR_InputDodgeComponent::OnMoveTriggered);
    InputComponent->BindAction(InputActionForward, ETriggerEvent::Triggered, this, &UUR_InputDodgeComponent::OnForwardTriggered);
    InputComponent->BindAction(InputActionBackward, ETriggerEvent::Triggered, this, &UUR_InputDodgeComponent::OnBackwardTriggered);
    InputComponent->BindAction(InputActionRight, ETriggerEvent::Triggered, this, &UUR_InputDodgeComponent::OnRightTriggered);
    InputComponent->BindAction(InputActionLeft, ETriggerEvent::Triggered, this, &UUR_InputDodgeComponent::OnLeftTriggered);
    InputComponent->BindAction(InputActionUp, ETriggerEvent::Triggered, this, &UUR_InputDodgeComponent::OnUpTriggered);
    InputComponent->BindAction(InputActionDown, ETriggerEvent::Triggered, this, &UUR_InputDodgeComponent::OnDownTriggered);
}

void UUR_InputDodgeComponent::OnKeyDodge(const FInputActionInstance& InputActionInstance)
{
    if (const auto Character = Cast<AUR_Character>(GetOwner()))
    {
        // Read cached MovementAxes to determine KeyDodge direction. Default to Forward
        if (CachedMove.Y > 0)
        {
            Character->SetDodgeDirection(EDodgeDirection::Right);
        }
        else if (CachedMove.Y < 0)
        {
            Character->SetDodgeDirection(EDodgeDirection::Left);
        }
        else if (CachedMove.X < 0.f)
        {
            Character->SetDodgeDirection(EDodgeDirection::Backward);
        }
        else
        {
            Character->SetDodgeDirection(EDodgeDirection::Forward);
        }
    }
}

void UUR_InputDodgeComponent::HandleTap(const FVector& Direction)
{
    const auto NewKeyTap = FKeyTapTime(Direction, GetWorld()->GetTimeSeconds());
    if (const auto Character = Cast<AUR_Character>(GetOwner()))
    {
        Character->SetDodgeDirection(LastKeyTap.GetDodgeDirection(NewKeyTap, MaxDodgeClickThresholdTime));
    }
    LastKeyTap = NewKeyTap;
}

void UUR_InputDodgeComponent::OnForwardTriggered(const FInputActionInstance& InputActionInstance)
{
   HandleTap(FVector::ForwardVector);
}

void UUR_InputDodgeComponent::OnBackwardTriggered(const FInputActionInstance& InputActionInstance)
{
    HandleTap(FVector::BackwardVector);
}

void UUR_InputDodgeComponent::OnRightTriggered(const FInputActionInstance& InputActionInstance)
{
    HandleTap(FVector::RightVector);
}

void UUR_InputDodgeComponent::OnLeftTriggered(const FInputActionInstance& InputActionInstance)
{
    HandleTap(FVector::LeftVector);
}

void UUR_InputDodgeComponent::OnUpTriggered(const FInputActionInstance& InputActionInstance)
{
    HandleTap(FVector::UpVector);
}

void UUR_InputDodgeComponent::OnDownTriggered(const FInputActionInstance& InputActionInstance)
{
    HandleTap(FVector::DownVector);
}

void UUR_InputDodgeComponent::OnMoveTriggered(const FInputActionInstance& InputActionInstance)
{
    CachedMove = InputActionInstance.GetValue().Get<FVector>();
}
