// Copyright Epic Games, Inc. All Rights Reserved.

#include "OpenTournamentTestsInputTestHelper.h"

void FOpenTournamentTestsPawnTestActions::ToggleCrouch()
{
	PerformAction(FToggleCrouchTestAction{});
}

void FOpenTournamentTestsPawnTestActions::PerformMelee()
{
	PerformAction(FMeleeTestAction{});
}

void FOpenTournamentTestsPawnTestActions::PerformJump()
{
	PerformAction(FJumpTestAction{});
}

void FOpenTournamentTestsPawnTestActions::MoveForward()
{
	PerformAxisAction(FMoveForwardTestAction{});
}

void FOpenTournamentTestsPawnTestActions::MoveBackward()
{
	PerformAxisAction(FMoveBackwardTestAction{});
}

void FOpenTournamentTestsPawnTestActions::StrafeLeft()
{
	PerformAxisAction(FStrafeLeftTestAction{});
}

void FOpenTournamentTestsPawnTestActions::StrafeRight()
{
	PerformAxisAction(FStrafeRightTestAction{});
}

void FOpenTournamentTestsPawnTestActions::RotateLeft()
{
	PerformAxisAction(FRotateLeftTestAction{});
}

void FOpenTournamentTestsPawnTestActions::RotateRight()
{
	PerformAxisAction(FRotateRightTestAction{});
}

void FOpenTournamentTestsPawnTestActions::PerformAxisAction(TFunction<void(const APawn* Pawn)> Action)
{
	// Perform move actions over the duration of 5 seconds
	PerformAction(Action, [this]() -> bool {
		if (StartTime.GetTicks() == 0)
		{
			StartTime = FDateTime::UtcNow();
		}

		FTimespan Elapsed = FDateTime::UtcNow() - StartTime;
		return Elapsed >= FTimespan::FromSeconds(5.0);
	});
}
