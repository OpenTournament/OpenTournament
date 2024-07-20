// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Components/ActorComponent.h"
#include "InputAction.h"
#include "UR_Type_DodgeDirection.h"
#include "UR_PCInputDodgeComponent.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations


/////////////////////////////////////////////////////////////////////////////////////////////////
// Structures

USTRUCT(BlueprintType)
struct FKeyTapTime
{
    GENERATED_USTRUCT_BODY()

    /**
    * Timestamp for last Left key tap
    */
    UPROPERTY(BluePrintReadOnly, Category = "Dodging")
    float LastTapLeftTime;

    /**
    * Timestamp for last Right key tap
    */
    UPROPERTY(BluePrintReadOnly, Category = "Dodging")
    float LastTapRightTime;

    /**
    * Timestamp for last Forward key tap
    */
    UPROPERTY(BluePrintReadOnly, Category = "Dodging")
    float LastTapForwardTime;

    /**
    * Timestamp for last Back key tap
    */
    UPROPERTY(BluePrintReadOnly, Category = "Dodging")
    float LastTapBackTime;

    /**
    * Timestamp for last Upwards key tap
    */
    UPROPERTY(BluePrintReadOnly, Category = "Dodging")
    float LastTapUpTime;

    /**
    * Timestamp for last Downwards key tap
    */
    UPROPERTY(BluePrintReadOnly, Category = "Dodging")
    float LastTapDownTime;

    FKeyTapTime()
        : LastTapLeftTime(0)
        , LastTapRightTime(0)
        , LastTapForwardTime(0)
        , LastTapBackTime(0)
        , LastTapUpTime(0)
        , LastTapDownTime(0)
    {
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////


/**
 *
 */
UCLASS(Config = Game, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OPENTOURNAMENT_API UUR_PCInputDodgeComponent : public UActorComponent
{
    GENERATED_BODY()

    UUR_PCInputDodgeComponent();

public:
    /**
    * Setup bindings for ActionMappings to functions
    */
    virtual void BeginPlay() override;

    /**
    * Handle Input for KeyDodging
    */
    virtual void ProcessPlayerInput(const float DeltaTime, const bool bGamePaused);

    /**
    * Current movement axis deflecton forward/back (back is negative)
    */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
    float MovementForwardAxis;

    /**
    * Current movement axis deflecton right/left (left is negative)
    */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
    float MovementStrafeAxis;

protected:

    /**
    * Set KeyDodge requested. Used to capture KeyDodge action and hold until MovementAxis values are valid.
    */
    UPROPERTY(BluePrintReadOnly, Category = "Dodging")
    bool bRequestedKeyDodge;

    /**
    * Max Time between directional keypresses that will register as a dodge
    */
    UPROPERTY(EditAnywhere, GlobalConfig, Category = "Dodging")
    float MaxDodgeClickThresholdTime;

    /**
    * Structure containing Key Tap times (Forward, Backward, Left, Right, Up, Down)
    */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Dodging")
    FKeyTapTime KeyTapTime;


    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InputActionKeyDodge;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InputActionTapForward;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InputActionTapBack;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InputActionTapRight;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InputActionTapLeft;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InputActionTapUp;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InputActionTapDown;

private:

    /**
    * Function bound to "KeyDodge" ActionMapping Input. Set bRequestedKeyDodge.
    */
    void OnKeyDodge(const FInputActionInstance& InputActionInstance);

    /**
    * Set the Dodge direction via cached MovementAxes.
    */
    void SetKeyDodgeInputDirection() const;

    /**
    * Function bound to "TapForward" ActionMapping Input.
    */
    void OnTapForward(const FInputActionInstance& InputActionInstance);

    /**
    * Function bound to "TapBack" ActionMapping Input.
    */
    void OnTapBack(const FInputActionInstance& InputActionInstance);

    /**
    * Function bound to "TapLeft" ActionMapping Input.
    */
    void OnTapLeft(const FInputActionInstance& InputActionInstance);

    /**
    * Function bound to "TapRight" ActionMapping Input.
    */
    void OnTapRight(const FInputActionInstance& InputActionInstance);

    /**
    * Function bound to "TapUpward" ActionMapping Input (Jump while 3DMovementMode).
    */
    void OnTapUp(const FInputActionInstance& InputActionInstance);

    /**
    * Function bound to "TapDownward" ActionMapping Input (Crouch while 3DMovementMode).
    */
    void OnTapDown(const FInputActionInstance& InputActionInstance);

    /**
     * Process Taps and set the Dodge direction.
     */
    void SetTapDodgeInputDirection(const float LastTapTime, const float MaxClickTime, const EDodgeDirection DodgeDirection) const;
};
