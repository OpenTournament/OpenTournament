// Copyright 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2019-2020 Open Tournament Project, All Rights Reserved.

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

public:

    UUR_InventoryComponent();

    UPROPERTY(EditAnywhere, Category = "InventoryComponent")
    TArray<AUR_Weapon*> InventoryW;

    UPROPERTY(EditAnywhere, Category = "InventoryComponent")
    TArray<AUR_Ammo*> InventoryA;

    UPROPERTY(EditAnywhere, Category = "InventoryComponent")
    AUR_Weapon * ActiveWeapon;

    void Add(AUR_Weapon* weapon);

    void Add(AUR_Ammo* ammo);

    void AmmoCountInInventory(AUR_Weapon* weapon);

    void UpdateWeaponAmmo(AUR_Ammo* ammo);

    UFUNCTION()
    void ShowInventory();

    UFUNCTION()
    int32 SelectWeapon(int32 number);

    UFUNCTION()
    AUR_Weapon * SelectWeaponG(int32 number);
};
