// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Components/ActorComponent.h"

#include "UR_Type_DodgeDirection.h"

#include "UR_InputDodgeComponent.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations

class UInputAction;
struct FInputActionInstance;

USTRUCT(BlueprintType)
struct FKeyTapTime
{
    GENERATED_BODY()

    FKeyTapTime()
    {
        std::fill(std::begin(InputTimes), std::end(InputTimes), 0);
    }

    FKeyTapTime(const FVector& Input, const float WorldTime)
    {
        InputTimes[0] = Input.X > 0 ? WorldTime : 0;
        InputTimes[1] = Input.X < 0 ? WorldTime : 0;
        InputTimes[2] = Input.Y > 0 ? WorldTime : 0;
        InputTimes[3] = Input.Y < 0 ? WorldTime : 0;
        InputTimes[4] = Input.Z > 0 ? WorldTime : 0;
        InputTimes[5] = Input.Z < 0 ? WorldTime : 0;
    }

    EDodgeDirection GetDodgeDirection(const FKeyTapTime& Other, const float MaxTapTime) const
    {
        for(int i = 0; i < 6; i++)
        {
            const auto TimeDifference = FMath::Abs(Other.InputTimes[i] - InputTimes[i]);
            if (InputTimes[i] > 0 && TimeDifference > 0 && FMath::Abs(Other.InputTimes[i] - InputTimes[i]) < MaxTapTime)
                return DodgeDirectionMapping[i];
        }

        return EDodgeDirection::None;
    }

private:

    EDodgeDirection DodgeDirectionMapping[6] = { EDodgeDirection::Forward, EDodgeDirection::Backward, EDodgeDirection::Right, EDodgeDirection::Left, EDodgeDirection::Up, EDodgeDirection::Down};
    float InputTimes[6];
};

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(Config = Game, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OPENTOURNAMENT_API UUR_InputDodgeComponent : public UActorComponent
{
    GENERATED_BODY()

    UUR_InputDodgeComponent();

public:

    /**
    * Setup bindings for ActionMappings to functions
    */
    void SetupInputComponent(class UUR_InputComponent* InputComponent);

protected:

    /**
    * Max Time between directional keypresses that will register as a dodge
    */
    UPROPERTY(EditAnywhere, GlobalConfig, Category = "Dodging")
    float MaxDodgeClickThresholdTime;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InputActionKeyDodge;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InputActionMove;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InputActionForward;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InputActionBackward;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InputActionRight;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InputActionLeft;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InputActionUp;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InputActionDown;
private:

    FKeyTapTime LastKeyTap;
    FVector CachedMove;

    void HandleTap(const FVector& Direction);
    void OnKeyDodge(const FInputActionInstance& InputActionInstance);
    void OnMoveTriggered(const FInputActionInstance& InputActionInstance);
    void OnForwardTriggered(const FInputActionInstance& InputActionInstance);
    void OnBackwardTriggered(const FInputActionInstance& InputActionInstance);
    void OnRightTriggered(const FInputActionInstance& InputActionInstance);
    void OnLeftTriggered(const FInputActionInstance& InputActionInstance);
    void OnUpTriggered(const FInputActionInstance& InputActionInstance);
    void OnDownTriggered(const FInputActionInstance& InputActionInstance);
};
