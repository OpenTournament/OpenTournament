// Copyright Epic Games, Inc.All Rights Reserved.

#include "Utilities/OpenTournamentTestsActorNetworkTest.h"

#if ENABLE_OpenTournamentTests_NETWORK_TEST

/**
 * Creates a standalone test object using the name from the first parameter, in the case `InputAnimationTest`, which inherits from `OpenTournamentTestsActorAnimationNetworkTest<Derived, AsserterType, FOpenTournamentTestsActorInputTestHelper>` to provide us our testing functionality.
 * The second parameter specifies the category and subcategories used for displaying within the UI
 *
 * Note that this test uses the ACTOR_ANIMATION_NETWORK_TEST macro, which is defined in `/Utilities/OpenTournamentTestsNetworkTest.h`, to inherit from a base class with user-defined variables and methods. Reference that document for more information.
 *
 * The test object will test animation playback during specific inputs while in a networked session. All variables are reset after each test iteration.
 *
 * The test makes use of the `TestCommandBuilder` to queue up latent commands to be executed on every Tick of the Engine/Editor
 * Because of the nature of a network session having a server and client, the `FOpenTournamentTestsNetworkComponent` handles splitting up the commands between the Server and Client PIE sessions.
 * `ThenServer` and `ThenClient` steps will execute within a single tick on the server or client
 * `UntilServer` and `UntilClient` steps will keep executing each tick on the server or client until the predicate has evaluated to true or the timeout period has elapsed. The latter will fail the test.
 *
 * Each TEST_METHOD will register with the `InputAnimationTest` test object and has the variables and methods from `InputAnimationTest` available for use.
 */

ACTOR_ANIMATION_NETWORK_TEST(InputAnimationTest, "Project.Functional Tests.OpenTournamentTests.Actor.Replication")
{
	// Make a call to our base Constructor to set level to load
	InputAnimationTest() : OpenTournamentTestsActorAnimationNetworkTest(TEXT("/OpenTournamentTests/Maps/L_ShooterTest_Basic"))
	{
	}

	/**
	 * Each test is registered with the name of the type of player being tested and checks both the expected animation and the input action used to trigger the animation
	 * If a test is focusing on the Server Player, the test name will be prefixed with `ServerPlayer`. Similarly, the Client Player tests are prefixed as `ClientPlayer`
	 * If a test is set to be performed on both the Server and CLient Players, then the prefix will be `NetworkPlayers`
	 * Helper methods are provided within `OpenTournamentTestsActorNetworkTest` to help streamline with the repeated functionality of performing inputs and fetching and testing animations on either the Server or Clients
	 */
	TEST_METHOD(NetworkPlayers_Jump)
	{
		FOpenTournamentTestsActorInputTestHelper::InputActionType Input = FOpenTournamentTestsActorInputTestHelper::InputActionType::Jump;

		// Test animation and input for the client instance
		FetchAnimationAssetForClientPlayer(FOpenTournamentTestsAnimationTestHelper::PistolJumpAnimationName);
		PerformInputOnClientPlayer(Input);
		IsClientPlayerAnimationPlayingOnAllClients();

		// Test animation and input for the server instance
		FetchAnimationAssetForServerPlayer(FOpenTournamentTestsAnimationTestHelper::PistolJumpAnimationName);
		PerformInputOnServerPlayer(Input);
		IsServerPlayerAnimationPlayingOnAllClients();
	}

	TEST_METHOD(ServerPlayer_MannyMovement)
	{
		// Map of the animations and their corresponding inputs to be tested against in the loop below.
		// Note that due to how the level is laid out, the server player spawns at the first found player location, while the client spawns to the next of them to avoid immediate collisions.
		// We take the spawn into account and have the server player test their directional movements by going left before going to the right to allow for some additional buffer before the animation starts playing.
		TMap<FString, FOpenTournamentTestsActorInputTestHelper::InputActionType> MovementAnimations
		{
			{ FOpenTournamentTestsAnimationTestHelper::MannyPistolJogForwardAnimationName, FOpenTournamentTestsActorInputTestHelper::InputActionType::MoveForward },
			{ FOpenTournamentTestsAnimationTestHelper::MannyPistolJogBackwardAnimationName, FOpenTournamentTestsActorInputTestHelper::InputActionType::MoveBackward },
			{ FOpenTournamentTestsAnimationTestHelper::MannyPistolStrafeLeftAnimationName, FOpenTournamentTestsActorInputTestHelper::InputActionType::StrafeLeft },
			{ FOpenTournamentTestsAnimationTestHelper::MannyPistolStrafeRightAnimationName, FOpenTournamentTestsActorInputTestHelper::InputActionType::StrafeRight },
		};

		for (const TPair<FString, FOpenTournamentTestsActorInputTestHelper::InputActionType>& MovementAnimation : MovementAnimations)
		{
			const FString& AnimationName = MovementAnimation.Key;
			FOpenTournamentTestsActorInputTestHelper::InputActionType Input = MovementAnimation.Value;

			FetchAnimationAssetForServerPlayer(AnimationName);
			PerformInputOnServerPlayer(Input);
			IsServerPlayerAnimationPlayingOnAllClients();
			StopAllInputsOnServerPlayer();
		}
	}

	TEST_METHOD(ClientPlayer_QuinnMovement)
	{
		// Map of the animations and their corresponding inputs to be tested against in the loop below.
		// Note that due to how the level is laid out, the server player spawns at the first found player location, while the client spawns to the next of them to avoid immediate collisions.
		// We take the spawn into account and have the client player test their directional movements by going right before going to the left to allow for some additional buffer before the animation starts playing.
		TMap<FString, FOpenTournamentTestsActorInputTestHelper::InputActionType> MovementAnimations
		{
			{ FOpenTournamentTestsAnimationTestHelper::QuinnPistolJogForwardAnimationName, FOpenTournamentTestsActorInputTestHelper::InputActionType::MoveForward },
			{ FOpenTournamentTestsAnimationTestHelper::QuinnPistolJogBackwardAnimationName, FOpenTournamentTestsActorInputTestHelper::InputActionType::MoveBackward },
			{ FOpenTournamentTestsAnimationTestHelper::QuinnPistolStrafeRightAnimationName, FOpenTournamentTestsActorInputTestHelper::InputActionType::StrafeRight },
			{ FOpenTournamentTestsAnimationTestHelper::QuinnPistolStrafeLeftAnimationName, FOpenTournamentTestsActorInputTestHelper::InputActionType::StrafeLeft },
		};

		for (const TPair<FString, FOpenTournamentTestsActorInputTestHelper::InputActionType>& MovementAnimation : MovementAnimations)
		{
			const FString& AnimationName = MovementAnimation.Key;
			FOpenTournamentTestsActorInputTestHelper::InputActionType Input = MovementAnimation.Value;

			FetchAnimationAssetForClientPlayer(AnimationName);
			PerformInputOnClientPlayer(Input);
			IsClientPlayerAnimationPlayingOnAllClients();
			StopAllInputsOnClientPlayer();
		}
	}

	TEST_METHOD(NetworkPlayers_Crouch)
	{
		FOpenTournamentTestsActorInputTestHelper::InputActionType Input = FOpenTournamentTestsActorInputTestHelper::InputActionType::Crouch;

		// Test animation and input for the client instance
		FetchAnimationAssetForClientPlayer(FOpenTournamentTestsAnimationTestHelper::PistolCrouchIdleAnimationName);
		PerformInputOnClientPlayer(Input);
		IsClientPlayerAnimationPlayingOnAllClients();

		// Test animation and input for the server instance
		FetchAnimationAssetForServerPlayer(FOpenTournamentTestsAnimationTestHelper::PistolCrouchIdleAnimationName);
		PerformInputOnServerPlayer(Input);
		IsServerPlayerAnimationPlayingOnAllClients();
	}
};

#endif // ENABLE_OpenTournamentTests_NETWORK_TEST
