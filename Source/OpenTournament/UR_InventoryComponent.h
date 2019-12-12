// Copyright 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Components/ActorComponent.h"

#include "OpenTournament.h"
#include "UR_Weapon.h"
#include "UR_Ammo.h"


#include "UR_InventoryComponent.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declaration


/////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * InventoryComponent is the base component for use by actors to have an inventory.
 */
UCLASS(DefaultToInstanced, BlueprintType, meta = (Tooltip = "A InventoryComponent is a reusable component that can be added to any actor to give it a Inventory value.", ShortTooltip = "A InventoryComponent is a reusable component that can be added to any actor to give it a Inventory value."), hideCategories = (UR, Character, Collision, Cooking))
class OPENTOURNAMENT_API UUR_InventoryComponent : public UActorComponent
{
    GENERATED_BODY()

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:

    UUR_InventoryComponent();

	UPROPERTY(ReplicatedUsing = OnRep_InventoryW, BlueprintReadOnly, Category = "InventoryComponent")
	TArray<AUR_Weapon*> InventoryW;

	UPROPERTY(BlueprintReadOnly, Category = "InventoryComponent")
	TArray<AUR_Ammo*> InventoryA;

	UPROPERTY(ReplicatedUsing = OnRep_ActiveWeapon, BlueprintReadOnly, Category = "InventoryComponent")
	AUR_Weapon * ActiveWeapon;

	void Add(AUR_Weapon* weapon);

	void Add(AUR_Ammo* ammo);

	void AmmoCountInInventory(AUR_Weapon* weapon);

	void UpdateWeaponAmmo(AUR_Ammo* ammo);

	UFUNCTION()
	void ShowInventory();

	UFUNCTION()
	int32 SelectWeapon(int number);

	UFUNCTION()
	AUR_Weapon * SelectWeaponG(int number);

	UFUNCTION()
	bool NextWeapon();

	UFUNCTION()
	bool PrevWeapon();

	UFUNCTION()
	void EquipWeapon(AUR_Weapon* Weap);

	UFUNCTION(Server, Reliable)
	void ServerEquipWeapon(AUR_Weapon* Weap);

	UFUNCTION()
	virtual void OnRep_InventoryW();

	UFUNCTION()
	virtual void OnRep_ActiveWeapon();
};
