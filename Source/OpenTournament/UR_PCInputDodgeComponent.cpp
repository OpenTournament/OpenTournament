// Copyright 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_PCInputDodgeComponent.h"

#include "Engine/World.h"

#include "UR_Character.h"
#include "UR_CharacterMovementComponent.h"
#include "UR_PlayerController.h"

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

void UUR_PCInputDodgeComponent::BeginPlay()
{
    if (const auto OwningPC = Cast<AUR_PlayerController>(GetOwner()))
    {
        if (auto InputComponent = OwningPC->InputComponent)
        {
            InputComponent->BindAction("KeyDodge", IE_Pressed, this, &UUR_PCInputDodgeComponent::OnKeyDodge);

            InputComponent->BindAction("TapLeft", IE_Pressed, this, &UUR_PCInputDodgeComponent::OnTapLeft);
            InputComponent->BindAction("TapRight", IE_Pressed, this, &UUR_PCInputDodgeComponent::OnTapRight);
            InputComponent->BindAction("TapForward", IE_Pressed, this, &UUR_PCInputDodgeComponent::OnTapForward);
            InputComponent->BindAction("TapBack", IE_Pressed, this, &UUR_PCInputDodgeComponent::OnTapBack);
            InputComponent->BindAction("TapUp", IE_Pressed, this, &UUR_PCInputDodgeComponent::OnTapUpward);
            InputComponent->BindAction("TapDown", IE_Pressed, this, &UUR_PCInputDodgeComponent::OnTapDownward);
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

void UUR_PCInputDodgeComponent::OnKeyDodge()
{
    bRequestedKeyDodge = true;
}

void UUR_PCInputDodgeComponent::SetKeyDodgeInputDirection() const
{
    if (const auto OwningPC = Cast<AUR_PlayerController>(GetOwner()))
    {
        if (const auto Character = Cast<AUR_Character>(OwningPC->GetCharacter()))
        {
            const auto MovementComponent = Character->URMovementComponent;

            if (MovementComponent != nullptr && !OwningPC->IsMoveInputIgnored())
            {
                // Zero out KeyDodge directionality
                MovementComponent->DodgeDirection = EDodgeDirection::DD_None;

                // Read cached MovementAxes to determine KeyDodge direction. Default to Forward
                if (MovementStrafeAxis > 0.5f)
                {
                    MovementComponent->DodgeDirection = EDodgeDirection::DD_Right;
                }
                else if (MovementStrafeAxis < -0.5f)
                {
                    MovementComponent->DodgeDirection = EDodgeDirection::DD_Left;
                }
                else if (MovementForwardAxis < 0.f)
                {
                    MovementComponent->DodgeDirection = EDodgeDirection::DD_Backward;
                }
                else
                {
                    MovementComponent->DodgeDirection = EDodgeDirection::DD_Forward;
                }
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_PCInputDodgeComponent::OnTapForward()
{
    KeyTapTime.LastTapRightTime = -10.f;
    KeyTapTime.LastTapLeftTime = -10.f;
    KeyTapTime.LastTapBackTime = -10.f;
    KeyTapTime.LastTapUpTime = -10.f;
    KeyTapTime.LastTapDownTime = -10.f;

    SetTapDodgeInputDirection(KeyTapTime.LastTapForwardTime, MaxDodgeClickThresholdTime, EDodgeDirection::DD_Forward);
    KeyTapTime.LastTapForwardTime = GetWorld()->GetTimeSeconds();
}

void UUR_PCInputDodgeComponent::OnTapBack()
{
    KeyTapTime.LastTapRightTime = -10.f;
    KeyTapTime.LastTapLeftTime = -10.f;
    KeyTapTime.LastTapForwardTime = -10.f;
    KeyTapTime.LastTapUpTime = -10.f;
    KeyTapTime.LastTapDownTime = -10.f;

    SetTapDodgeInputDirection(KeyTapTime.LastTapBackTime, MaxDodgeClickThresholdTime, EDodgeDirection::DD_Backward);
    KeyTapTime.LastTapBackTime = GetWorld()->GetTimeSeconds();
}

void UUR_PCInputDodgeComponent::OnTapLeft()
{
    KeyTapTime.LastTapRightTime = -10.f;
    KeyTapTime.LastTapForwardTime = -10.f;
    KeyTapTime.LastTapBackTime = -10.f;
    KeyTapTime.LastTapUpTime = -10.f;
    KeyTapTime.LastTapDownTime = -10.f;

    SetTapDodgeInputDirection(KeyTapTime.LastTapLeftTime, MaxDodgeClickThresholdTime, EDodgeDirection::DD_Left);
    KeyTapTime.LastTapLeftTime = GetWorld()->GetTimeSeconds();
}

void UUR_PCInputDodgeComponent::OnTapRight()
{
    KeyTapTime.LastTapLeftTime = -10.f;
    KeyTapTime.LastTapForwardTime = -10.f;
    KeyTapTime.LastTapBackTime = -10.f;
    KeyTapTime.LastTapUpTime = -10.f;
    KeyTapTime.LastTapDownTime = -10.f;

    SetTapDodgeInputDirection(KeyTapTime.LastTapRightTime, MaxDodgeClickThresholdTime, EDodgeDirection::DD_Right);
    KeyTapTime.LastTapRightTime = GetWorld()->GetTimeSeconds();
}

void UUR_PCInputDodgeComponent::OnTapUpward()
{
    KeyTapTime.LastTapRightTime = -10.f;
    KeyTapTime.LastTapLeftTime = -10.f;
    KeyTapTime.LastTapForwardTime = -10.f;
    KeyTapTime.LastTapBackTime = -10.f;
    KeyTapTime.LastTapDownTime = -10.f;

    SetTapDodgeInputDirection(KeyTapTime.LastTapUpTime, MaxDodgeClickThresholdTime, EDodgeDirection::DD_Up);
    KeyTapTime.LastTapUpTime = GetWorld()->GetTimeSeconds();
}

void UUR_PCInputDodgeComponent::OnTapDownward()
{
    KeyTapTime.LastTapRightTime = -10.f;
    KeyTapTime.LastTapLeftTime = -10.f;
    KeyTapTime.LastTapForwardTime = -10.f;
    KeyTapTime.LastTapBackTime = -10.f;
    KeyTapTime.LastTapUpTime = -10.f;

    SetTapDodgeInputDirection(KeyTapTime.LastTapDownTime, MaxDodgeClickThresholdTime, EDodgeDirection::DD_Down);
    KeyTapTime.LastTapDownTime = GetWorld()->GetTimeSeconds();
}

void UUR_PCInputDodgeComponent::OnTapForwardRelease()
{
    // SingleTap WallDodge Behavior
}

void UUR_PCInputDodgeComponent::OnTapBackRelease()
{

}

void UUR_PCInputDodgeComponent::OnTapLeftRelease()
{

}

void UUR_PCInputDodgeComponent::OnTapRightRelease()
{

}

void UUR_PCInputDodgeComponent::SetTapDodgeInputDirection(const float LastTapTime, const float MaxClickTime,
                                                       const EDodgeDirection DodgeDirection) const
{
    if (const auto OwningPC = Cast<AUR_PlayerController>(GetOwner()))
    {
        if (const auto Character = Cast<AUR_Character>(OwningPC->GetCharacter()))
        {
            const auto MovementComponent = Character->URMovementComponent;
            if (MovementComponent != nullptr && !OwningPC->IsMoveInputIgnored())
            {
                if (GetWorld()->GetTimeSeconds() - LastTapTime < MaxClickTime)
                {
                    MovementComponent->DodgeDirection = DodgeDirection;
                }
            }
        }
    }
}
