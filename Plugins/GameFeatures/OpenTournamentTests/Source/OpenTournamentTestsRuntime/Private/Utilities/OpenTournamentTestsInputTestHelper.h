// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/InputTestActions.h"
#include "Misc/DateTime.h"

/**
 * Input action to handle the Game player's crouch action.
 *
 * @note Crouching is handled by a button press which is a boolean value.
 */
struct FToggleCrouchTestAction : public FTestAction
{
	FToggleCrouchTestAction()
	{
		InputActionName = TEXT("IA_Crouch");
		InputActionValue = FInputActionValue(true);
	}
};

/**
 * Input action to handle the Game player's melee action.
 *
 * @note Melee is handled by a button press which is a boolean value.
 */
struct FMeleeTestAction : public FTestAction
{
	FMeleeTestAction()
	{
		InputActionName = TEXT("IA_Melee");
		InputActionValue = FInputActionValue(true);
	}
};

/**
 * Input action to handle the Game player's jump action.
 *
 * @note Jump is handled by a button press which is a boolean value.
 */
struct FJumpTestAction : public FTestAction
{
	FJumpTestAction()
	{
		InputActionName = TEXT("IA_Jump");
		InputActionValue = FInputActionValue(true);
	}
};

/**
 * Base input action to handle the Game player's movement.
 *
 * @note Movement is setup as a 2D axis with the X-axis handling strafing and the Y-axis handling moving forward/backward.
 * @note Derived objects below will handle movement direction along a particular axis
 */
struct FMoveTestAction : public FTestAction
{
	FMoveTestAction(const FInputActionValue& InInputActionValue)
	{
		InputActionName = TEXT("IA_Move");
		InputActionValue = InInputActionValue;
	}
};

/**
 * Base input action to handle the Game player's look (rotation).
 *
 * @note Rotation is setup as a 2D axis
 * @note Derived objects below will handle rotation along a particular axis
 */
struct FLookTestAction : public FTestAction
{
	FLookTestAction(const FInputActionValue& InInputActionValue)
	{
		InputActionName = TEXT("IA_Look_Mouse");
		InputActionValue = InInputActionValue;
	}
};

/** Movement input action to move the Game player forward. */
struct FMoveForwardTestAction : public FMoveTestAction
{
	FMoveForwardTestAction() : FMoveTestAction(FVector2D(0.0f, 1.0f))
	{
	}
};

/** Movement input action to move the Game player backward. */
struct FMoveBackwardTestAction : public FMoveTestAction
{
	FMoveBackwardTestAction() : FMoveTestAction(FVector2D(0.0f, -1.0f))
	{
	}
};

/** Movement input action to strafe the Game player to the left. */
struct FStrafeLeftTestAction : public FMoveTestAction
{
	FStrafeLeftTestAction() : FMoveTestAction(FVector2D(-1.0f, 0.0f))
	{
	}
};

/** Movement input action to strafe the Game player to the right. */
struct FStrafeRightTestAction : public FMoveTestAction
{
	FStrafeRightTestAction() : FMoveTestAction(FVector2D(1.0f, 0.0f))
	{
	}
};

/** Rotation input action to rotate the Game player to the left. */
struct FRotateLeftTestAction : public FLookTestAction
{
	FRotateLeftTestAction() : FLookTestAction(FVector2D(-1.0f, 0.0f))
	{
	}
};

/** Rotation input action to rotate the Game player to the right. */
struct FRotateRightTestAction : public FLookTestAction
{
	FRotateRightTestAction() : FLookTestAction(FVector2D(1.0f, 0.0f))
	{
	}
};

/**
 * Inherited InputTestAction used for testing our button and axis interactions for the Game player.
 *
 * @see FInputTestActions
 */
class FOpenTournamentTestsPawnTestActions : public FInputTestActions
{
public:
	/**
	 * Construct our Input actions object.
	 *
	 * @param Pawn - Pawn which will have the input actions applied against.
	 */
	explicit FOpenTournamentTestsPawnTestActions(APawn* Pawn) : FInputTestActions(Pawn)
	{
	}

	/** Simulate a button press for our crouch action. */
	void ToggleCrouch();

	/** Simulate a button press for our melee action. */
	void PerformMelee();

	/** Simulate a button press for our jump action. */
	void PerformJump();

	/** Simulates player movement input actions. */
	void MoveForward();
	void MoveBackward();

	/** Simulates player strafing input actions. */
	void StrafeLeft();
	void StrafeRight();

	/** Simulates player look/rotation input actions. */
	void RotateLeft();
	void RotateRight();

private:
	/** Method to perform any of our axis based actions over the span of 5 seconds. */
	void PerformAxisAction(TFunction<void(const APawn* Pawn)> Action);

	FDateTime StartTime{ 0 };
};
