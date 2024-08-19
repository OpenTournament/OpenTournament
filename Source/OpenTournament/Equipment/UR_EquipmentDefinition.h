// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Templates/SubclassOf.h"

#include "UR_EquipmentDefinition.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AActor;
class UUR_AbilitySet;
class UUR_EquipmentInstance;

/////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct FUR_EquipmentActorToSpawn
{
    GENERATED_BODY()

    FUR_EquipmentActorToSpawn()
    {
    }

    UPROPERTY(EditAnywhere, Category=Equipment)
    TSubclassOf<AActor> ActorToSpawn;

    UPROPERTY(EditAnywhere, Category=Equipment)
    FName AttachSocket;

    UPROPERTY(EditAnywhere, Category=Equipment)
    FTransform AttachTransform;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * UUR_EquipmentDefinition
 *
 * Definition of a piece of equipment that can be applied to a pawn
 */
UCLASS(Blueprintable, Const, Abstract, BlueprintType)
class UUR_EquipmentDefinition : public UObject
{
    GENERATED_BODY()

public:
    UUR_EquipmentDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    // Class to spawn
    UPROPERTY(EditDefaultsOnly, Category=Equipment)
    TSubclassOf<UUR_EquipmentInstance> InstanceType;

    // Gameplay ability sets to grant when this is equipped
    UPROPERTY(EditDefaultsOnly, Category=Equipment)
    TArray<TObjectPtr<const UUR_AbilitySet>> AbilitySetsToGrant;

    // Actors to spawn on the pawn when this is equipped
    UPROPERTY(EditDefaultsOnly, Category=Equipment)
    TArray<FUR_EquipmentActorToSpawn> ActorsToSpawn;
};
