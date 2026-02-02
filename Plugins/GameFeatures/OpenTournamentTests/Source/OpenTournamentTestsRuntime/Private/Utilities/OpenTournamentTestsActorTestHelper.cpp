// Copyright Epic Games, Inc. All Rights Reserved.

#include "OpenTournamentTestsActorTestHelper.h"

#include "Character/UR_Character.h"
#include "Components/SkeletalMeshComponent.h"

FOpenTournamentTestsActorTestHelper::FOpenTournamentTestsActorTestHelper(APawn* Pawn)
{
	checkf(Pawn, TEXT("Pawn is invalid."));

	GameCharacter = Cast<AUR_Character>(Pawn);
	checkf(GameCharacter, TEXT("Cannot cast Pawn to a Game Character."));

	UActorComponent* ActorComponent = GameCharacter->GetComponentByClass(USkeletalMeshComponent::StaticClass());
	checkf(ActorComponent, TEXT("Cannot find SkeletalMeshComponent from the UR_Character."));

	SkeletalMeshComponent = Cast<USkeletalMeshComponent>(ActorComponent);
	checkf(SkeletalMeshComponent, TEXT("Cannot cast component to SkeletalMeshComponent."));

	AbilitySystemComponent = GameCharacter->GetGameAbilitySystemComponent();
	checkf(AbilitySystemComponent, TEXT("Game Character does not have a valid AbilitySystemComponent."));

	const FName CharacterSpawn = TEXT("GameplayCue.Character.Spawn"); // @! TODO: Use Natively Declared Definition
	GameplayCueCharacterSpawnTag = FGameplayTag::RequestGameplayTag(CharacterSpawn, true);
}

bool FOpenTournamentTestsActorTestHelper::IsPawnFullySpawned()
{
	bool bIsCurrentlySpawning = AbilitySystemComponent->IsGameplayCueActive(GameplayCueCharacterSpawnTag);
	return !bIsCurrentlySpawning;
}

FOpenTournamentTestsActorInputTestHelper::FOpenTournamentTestsActorInputTestHelper(APawn* Pawn) : FOpenTournamentTestsActorTestHelper(Pawn)
{
	PawnActions = MakeUnique<FOpenTournamentTestsPawnTestActions>(Pawn);
}

void FOpenTournamentTestsActorInputTestHelper::PerformInput(InputActionType Type)
{
	switch (Type)
	{
		case InputActionType::Crouch:
			PawnActions->ToggleCrouch();
			break;
		case InputActionType::Melee:
			PawnActions->PerformMelee();
			break;
		case InputActionType::Jump:
			PawnActions->PerformJump();
			break;
		case InputActionType::MoveForward:
			PawnActions->MoveForward();
			break;
		case InputActionType::MoveBackward:
			PawnActions->MoveBackward();
			break;
		case InputActionType::StrafeLeft:
			PawnActions->StrafeLeft();
			break;
		case InputActionType::StrafeRight:
			PawnActions->StrafeRight();
			break;
		default:
			checkNoEntry();
	}
}

void FOpenTournamentTestsActorInputTestHelper::StopAllInput()
{
	PawnActions->StopAllActions();
}
