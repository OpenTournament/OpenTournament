// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "OpenTournamentTestsNetworkComponent.h"

#if ENABLE_OpenTournamentTests_NETWORK_TEST

#include "CQTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "Utilities/OpenTournamentTestsAnimationTestHelper.h"

/**
 * Implementation of our base class used to share functionality of loading a level and players over a network.
 * Inherits from `TTest<Derived, AsserterType>` to provide us our testing functionality.
 * Takes in an optional templated typename for the `FOpenTournamentTestsActorTestHelper` object and all possible inherited objects.
 *
 * Implements functionality to load a level, specified as parameters in the Constructor, and verifies that both the server and client Players are completely spawned within the game world during the `Setup`
 *
 * Makes use of the `TestCommandBuilder` to queue up latent commands to be executed on every Tick of the Engine/Editor
 * `Then` steps will execute within a single tick
 * `Until` steps will keep executing each tick until the predicate has evaluated to true or the timeout period has elapsed. The latter will fail the test.
 * Each step is defined and implemented to handle latent commands on both the server and client states.
 */
template<typename Derived, typename AsserterType, typename NetworkActorType = FOpenTournamentTestsActorTestHelper>
struct OpenTournamentTestsBaseActorNetworkTest : public TTest<Derived, AsserterType>
{
	/** Let this object know about our templated parent's variables */
	using TTest<Derived, AsserterType>::TestRunner;
	using TTest<Derived, AsserterType>::Assert;
	using TTest<Derived, AsserterType>::TestCommandBuilder;

	/**
	 * Construct the Actor Network Test.
	 *
	 * @param InMapName - The full package path of the map to load.
	 */
	OpenTournamentTestsBaseActorNetworkTest(const FString& InMapName) : MapName(InMapName)
	{
		static_assert(std::is_convertible_v<NetworkActorType*, FOpenTournamentTestsActorTestHelper*>, "NetworkActorType must derive from FOpenTournamentTestsActorTestHelper");
	}

	/** Setup the test by loading in the specified level and making sure that both the server and client Game players are fully spawned in before continuing. */
	virtual void Setup() override
	{
		FAutomationEditorCommonUtils::LoadMap(MapName);

		Network
			.Start()
			.PrepareAndWaitForServerPlayerSpawn()
			.PrepareAndWaitForClientPlayerSpawn()
			.FetchConnectedPlayerOnServer()
			.FetchConnectedPlayerOnClient();
	}

	/** Networking component which handles our client/server latent commands. */
	FOpenTournamentTestsNetworkComponent<NetworkActorType> Network{ TestRunner, TestCommandBuilder, TestRunner->bInitializing };

	/** Map package name that will be loaded for testing. */
	FString MapName;
};

/**
 * Implementation of our base class used to share functionality of loading a level and players over a network.
 * Inherits from `OpenTournamentTestsActorBaseTest<Derived, AsserterType, NetworkActorType>` to provide us our testing functionality and to handle our initial World setup.
 * Specifies the `NetworkActorType` to be of type `FOpenTournamentTestsActorInputTestHelper` which has extra functionality for Input handling.
 *
 * Apart from loading our level and Players from the base `OpenTournamentTestsBaseActorNetworkTest` object, will set up the FOpenTournamentTestsPawnTestActions object for input handling.
 * FOpenTournamentTestsPawnTestActions is a user-defined input handling object which derives from CQTest's `FInputTestActions` in order to specify Input Actions around what is available to our Player.
 *
 * Makes use of the `TestCommandBuilder` to queue up latent commands to be executed on every Tick of the Engine/Editor
 * `Then` steps will execute within a single tick
 * `Until` steps will keep executing each tick until the predicate has evaluated to true or the timeout period has elapsed. The latter will fail the test.
 * Each step is defined and implemented to handle latent commands on both the server and client states.
 */
template<typename Derived, typename AsserterType>
struct OpenTournamentTestsActorAnimationNetworkTest : public OpenTournamentTestsBaseActorNetworkTest<Derived, AsserterType, FOpenTournamentTestsActorInputTestHelper>
{
	/** Let this object know about our templated parent's variables */
	using OpenTournamentTestsBaseActorNetworkTest<Derived, AsserterType, FOpenTournamentTestsActorInputTestHelper>::Network;
	using TTest<Derived, AsserterType>::TestRunner;
	using TTest<Derived, AsserterType>::Assert;
	using TTest<Derived, AsserterType>::TestCommandBuilder;

	/**
	 * Construct the Actor Network Test.
	 *
	 * @param MapName - The full package path of the map to load.
	 */
	OpenTournamentTestsActorAnimationNetworkTest(const FString& InMapName) : OpenTournamentTestsBaseActorNetworkTest<Derived, AsserterType, FOpenTournamentTestsActorInputTestHelper>(InMapName)
	{
	}

	/**
	 * Search for a UAnimationAssest for the server player and verify that the animation exists on the connected client.
	 *
	 * @param AnimationName - Name of the animation to search for.
	 *
	 * @note This method will set the `ExpectedAnimation` to the found UAnimationAssest.
	 */
	void FetchAnimationAssetForServerPlayer(const FString& AnimationName)
	{
		FString ServerDescription = FString::Format(TEXT("Fetching animation asset '{0}' from the server player."), { AnimationName });
		FString ClientDescription = FString::Format(TEXT("Validating animation asset '{0}' from the connected player on the client."), { AnimationName });

		Network
			.ThenServer(*ServerDescription, [this, AnimationName](FOpenTournamentTestsNetworkState<FOpenTournamentTestsActorInputTestHelper>& ServerState) {
				ExpectedAnimation = AnimationTestHelper.FindAnimationAsset(ServerState.LocalPlayer->GetSkeletalMeshComponent(), AnimationName);
				ASSERT_THAT(IsNotNull(ExpectedAnimation, TEXT("No animation found in player.")));
			})
			.ThenClient(*ClientDescription, [this](FOpenTournamentTestsNetworkState<FOpenTournamentTestsActorInputTestHelper>& ClientState) {
				FString AnimationName = ExpectedAnimation->GetName();
				UAnimationAsset* AnimationAsset = AnimationTestHelper.FindAnimationAsset(ClientState.NetworkPlayer->GetSkeletalMeshComponent(), AnimationName);
				ASSERT_THAT(IsNotNull(AnimationAsset, TEXT("No animation found in connected player.")));
			});
	}

	/**
	 * Search for a UAnimationAssest for the client player and verify that the animation exists on the connected client.
	 *
	 * @param AnimationName - Name of the animation to search for.
	 *
	 * @note This method will set the `ExpectedAnimation` to the found UAnimationAssest.
	 */
	void FetchAnimationAssetForClientPlayer(const FString& AnimationName)
	{
		FString ServerDescription = FString::Format(TEXT("Fetching animation asset '{0}' from the connected player on the server."), { AnimationName });
		FString ClientDescription = FString::Format(TEXT("Validating animation asset '{0}' on the local client player."), { AnimationName });

		Network
			.ThenServer(*ServerDescription, [this, AnimationName](FOpenTournamentTestsNetworkState<FOpenTournamentTestsActorInputTestHelper>& ServerState) {
				ExpectedAnimation = AnimationTestHelper.FindAnimationAsset(ServerState.NetworkPlayer->GetSkeletalMeshComponent(), AnimationName);
				ASSERT_THAT(IsNotNull(ExpectedAnimation, TEXT("No animation found in player.")));
			})
			.ThenClient(*ClientDescription, [this](FOpenTournamentTestsNetworkState<FOpenTournamentTestsActorInputTestHelper>& ClientState) {
				FString AnimationName = ExpectedAnimation->GetName();
				UAnimationAsset* AnimationAsset = AnimationTestHelper.FindAnimationAsset(ClientState.LocalPlayer->GetSkeletalMeshComponent(), AnimationName);
				ASSERT_THAT(IsNotNull(AnimationAsset, TEXT("No animation found in player.")));
			});
	}

	/**
	 * Perform an input on the server's Game character.
	 *
	 * @param Type - Type of input to be performed.
	 */
	void PerformInputOnServerPlayer(FOpenTournamentTestsActorInputTestHelper::InputActionType Type)
	{
		Network.ThenServer(TEXT("Performing the input action on the local server player."), [this, Type](FOpenTournamentTestsNetworkState<FOpenTournamentTestsActorInputTestHelper>& ServerState) {
			ServerState.LocalPlayer->PerformInput(Type);
		});
	}

	/**
	 * Perform an input on the client's Game character.
	 *
	 * @param Type - Type of input to be performed.
	 */
	void PerformInputOnClientPlayer(FOpenTournamentTestsActorInputTestHelper::InputActionType Type)
	{
		Network.ThenClient(TEXT("Performing input action on the local client player."), [this, Type](FOpenTournamentTestsNetworkState<FOpenTournamentTestsActorInputTestHelper>& ClientState) {
			ClientState.LocalPlayer->PerformInput(Type);
		});
	}

	/** Stops any actively running inputs on the server's Game character. */
	void StopAllInputsOnServerPlayer()
	{
		Network.ThenServer(TEXT("Stopping all input actions on the local server player."), [this](FOpenTournamentTestsNetworkState<FOpenTournamentTestsActorInputTestHelper>& ServerState) {
			ServerState.LocalPlayer->StopAllInput();
		});
	}

	/** Stops any actively running inputs on the client's Game character. */
	void StopAllInputsOnClientPlayer()
	{
		Network.ThenClient(TEXT("Stopping all input actions on the local client player."), [this](FOpenTournamentTestsNetworkState<FOpenTournamentTestsActorInputTestHelper>& ClientState) {
			ClientState.LocalPlayer->StopAllInput();
		});
	}

	/** Check that the Game character representing the server player has the expected animation playing on all PIE sessions. */
	void IsServerPlayerAnimationPlayingOnAllClients()
	{
		Network
			.UntilServer(TEXT("Check if the local player on the server is playing the expected animation."), [this](FOpenTournamentTestsNetworkState<FOpenTournamentTestsActorInputTestHelper>& ServerState) {
				return AnimationTestHelper.IsAnimationPlaying(ServerState.LocalPlayer->GetSkeletalMeshComponent(), ExpectedAnimation);
			})
			.UntilClient(TEXT("Check if the connected player on the client is playing the expected animation."), [this](FOpenTournamentTestsNetworkState<FOpenTournamentTestsActorInputTestHelper>& ClientState) {
				return AnimationTestHelper.IsAnimationPlaying(ClientState.NetworkPlayer->GetSkeletalMeshComponent(), ExpectedAnimation);
			});
	}

	/** Check that the Game character representing the client player has the expected animation playing on all PIE sessions. */
	void IsClientPlayerAnimationPlayingOnAllClients()
	{
		Network
			.UntilClient(TEXT("Check if the local player on the client is playing the expected animation."), [this](FOpenTournamentTestsNetworkState<FOpenTournamentTestsActorInputTestHelper>& ClientState) {
				return AnimationTestHelper.IsAnimationPlaying(ClientState.LocalPlayer->GetSkeletalMeshComponent(), ExpectedAnimation);
			})
			.UntilServer(TEXT("Check if the connected player on the server is playing the expected animation."), [this](FOpenTournamentTestsNetworkState<FOpenTournamentTestsActorInputTestHelper>& ServerState) {
				return AnimationTestHelper.IsAnimationPlaying(ServerState.NetworkPlayer->GetSkeletalMeshComponent(), ExpectedAnimation);
			});
	}

	/** Animation helper object. */
	FOpenTournamentTestsAnimationTestHelper AnimationTestHelper;

	/** Reference to our animation asset. */
	UAnimationAsset* ExpectedAnimation{ nullptr };
};

/** Macro to quickly create network tests based on the above test object to only run within the Editor. */
#define ACTOR_ANIMATION_NETWORK_TEST(_ClassName, _TestDir) TEST_CLASS_WITH_BASE_AND_FLAGS(_ClassName, _TestDir, OpenTournamentTestsActorAnimationNetworkTest, EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

#endif // ENABLE_OpenTournamentTests_NETWORK_TEST
