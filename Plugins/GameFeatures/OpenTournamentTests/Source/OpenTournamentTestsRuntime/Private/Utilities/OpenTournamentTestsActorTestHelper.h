// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/UR_AbilitySystemComponent.h"
#include "OpenTournamentTestsInputTestHelper.h"

class AUR_Character;
class USkeletalMeshComponent;

/// Class which consolidates the Game Actor information that is shared amongst tests.
class FOpenTournamentTestsActorTestHelper
{
public:
	/**
	* Construct the Actor Test Helper object
	*
	* @param Pawn - Pointer to a Pawn
	*/
	explicit FOpenTournamentTestsActorTestHelper(APawn* Pawn);

	/**
	* Checks to see if the current actor is fully spawned in the level and ready to be used.
	*
	* @return true is the player is spawned and usable, otherwise false
	*/
	bool IsPawnFullySpawned();

	/**
	* Gets the Game character which was associated with the Pawn used during the construction of the object.
	*
	* @return constant pointer to the UR_Character
	*/
	const AUR_Character* GetGameCharacter() const { return GameCharacter; }

	/**
	* Gets the skeletal mesh component of the associated Game Character.
	*
	* @return pointer to the SkeletalMeshComponent
	*/
	USkeletalMeshComponent* GetSkeletalMeshComponent() const { return SkeletalMeshComponent; }

private:
	/** Reference to our player in the level. */
	AUR_Character* GameCharacter{ nullptr };

	/** Reference to the player's skeletal mesh component. */
	USkeletalMeshComponent* SkeletalMeshComponent;

	/** Reference to the player's ability system component. */
	UUR_AbilitySystemComponent* AbilitySystemComponent{ nullptr };

	/** Reference to the player's spawning gameplay effect. */
	FGameplayTag GameplayCueCharacterSpawnTag;
};

/**
 * Inherited from FOpenTournamentTestsActorTestHelper, adds FOpenTournamentTestsPawnTestActions to be used for interacting with the Game player and triggering animations.
 *
 * @see FOpenTournamentTestsActorTestHelper
 */
class FOpenTournamentTestsActorInputTestHelper : public FOpenTournamentTestsActorTestHelper
{
public:
	/** Defines the available input actions that can be performed. */
	enum class InputActionType : uint8_t
	{
		Crouch,
		Melee,
		Jump,
		MoveForward,
		MoveBackward,
		StrafeLeft,
		StrafeRight,
	};

	explicit FOpenTournamentTestsActorInputTestHelper(APawn* Pawn);

	/**
	* Simulates input triggers on the Game character.
	*
	* @param Type - InputActionType used to specify which input to perform.
	*/
	void PerformInput(InputActionType Type);

	/**
	* Stops all actively running inputs.
	*/
	void StopAllInput();

private:
	/** Object which handles interfacing with the Enhanced Input System to perform input actions. */
	TUniquePtr<FOpenTournamentTestsPawnTestActions> PawnActions;
};
