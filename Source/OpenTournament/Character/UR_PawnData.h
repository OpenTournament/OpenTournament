// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Engine/DataAsset.h"

#include "UR_PawnData.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class APawn;
class ULyraAbilitySet;
class ULyraAbilityTagRelationshipMapping;
class ULyraCameraMode;
class ULyraInputConfig;
class UObject;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * UUR_PawnData
 *
 *	Non-mutable data asset that contains properties used to define a pawn.
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "Game Pawn Data", ShortTooltip = "Data asset used to define a Pawn."))
class OPENTOURNAMENT_API UUR_PawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UUR_PawnData(const FObjectInitializer& ObjectInitializer);

public:

	// Class to instantiate for this pawn (should usually derive from ALyraPawn or ALyraCharacter).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Pawn")
	TSubclassOf<APawn> PawnClass;

    // @! TODO Add additional PawnData (See: Lyra)

	// // Ability sets to grant to this pawn's ability system.
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Abilities")
	// TArray<TObjectPtr<ULyraAbilitySet>> AbilitySets;
	//
	// // What mapping of ability tags to use for actions taking by this pawn
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Abilities")
	// TObjectPtr<ULyraAbilityTagRelationshipMapping> TagRelationshipMapping;
	//
	// // Input configuration used by player controlled pawns to create input mappings and bind input actions.
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Input")
	// TObjectPtr<ULyraInputConfig> InputConfig;
	//
	// // Default camera mode used by player controlled pawns.
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Camera")
	// TSubclassOf<ULyraCameraMode> DefaultCameraMode;
};
