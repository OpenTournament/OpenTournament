// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Engine/DataAsset.h"

#include "UR_PickupDefinition.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UUR_InventoryItemDefinition;
class UNiagaraSystem;
class UObject;
class USoundBase;
class UStaticMesh;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
UCLASS(Blueprintable, BlueprintType, Const, Meta = (DisplayName = "UR_ Pickup Data", ShortTooltip = "Data asset used to configure a pickup."))
class OPENTOURNAMENT_API UUR_PickupDefinition : public UDataAsset
{
    GENERATED_BODY()

public:
    //Defines the pickup's actors to spawn, abilities to grant, and tags to add
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OT|Pickup|Equipment")
    TSubclassOf<UUR_InventoryItemDefinition> InventoryItemDefinition;

    //Visual representation of the pickup
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OT|Pickup|Mesh")
    TObjectPtr<UStaticMesh> DisplayMesh;

    //Cool down time between pickups in seconds
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OT|Pickup")
    int32 SpawnCoolDownSeconds;

    //Sound to play when picked up
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OT|Pickup")
    TObjectPtr<USoundBase> PickedUpSound;

    //Sound to play when pickup is respawned
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OT|Pickup")
    TObjectPtr<USoundBase> RespawnedSound;

    //Particle FX to play when picked up
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OT|Pickup")
    TObjectPtr<UNiagaraSystem> PickedUpEffect;

    //Particle FX to play when pickup is respawned
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OT|Pickup")
    TObjectPtr<UNiagaraSystem> RespawnedEffect;
};


UCLASS(Blueprintable, BlueprintType, Const, Meta = (DisplayName = "OT Weapon Pickup Data", ShortTooltip = "Data asset used to configure a weapon pickup."))
class OPENTOURNAMENT_API UUR_WeaponPickupDefinition : public UUR_PickupDefinition
{
    GENERATED_BODY()

public:
    //Sets the height of the display mesh above the Weapon spawner
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OT|Pickup|Mesh")
    FVector WeaponMeshOffset;

    //Sets the height of the display mesh above the Weapon spawner
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OT|Pickup|Mesh")
    FVector WeaponMeshScale = FVector(1.0f, 1.0f, 1.0f);
};
