// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "UR_InventoryComponent.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declaration

class AUR_Ammo;
class AUR_Weapon;

/////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * InventoryComponent is the base component for use by actors to have an inventory.
 */
UCLASS(DefaultToInstanced, BlueprintType, meta = (Tooltip = "A InventoryComponent is a reusable component that can be added to any actor to give it an Inventory.", ShortTooltip = "A InventoryComponent is a reusable component that can be added to any actor to give it an Inventory."), hideCategories = (UR, Character, Collision, Cooking))
class OPENTOURNAMENT_API UUR_InventoryComponent : public UActorComponent
{
    GENERATED_BODY()

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

public:

    UUR_InventoryComponent();

    /////////////////////////////////////////////////////////////////////////////////////////////////

    UPROPERTY(ReplicatedUsing = OnRep_InventoryW, BlueprintReadOnly, Category = "InventoryComponent")
    TArray<AUR_Weapon*> InventoryW;

    UPROPERTY(BlueprintReadOnly, Category = "InventoryComponent")
    TArray<AUR_Ammo*> InventoryA;

    UPROPERTY(ReplicatedUsing = OnRep_ActiveWeapon, BlueprintReadOnly, Category = "InventoryComponent")
    AUR_Weapon * ActiveWeapon;

    void Add(AUR_Weapon* InWeapon);

    void Add(AUR_Ammo* InAmmo);

    void AmmoCountInInventory(AUR_Weapon* InWeapon);

    void UpdateWeaponAmmo(AUR_Ammo* InAmmo);

    UFUNCTION()
    void ShowInventory();

    UFUNCTION()
    int32 SelectWeapon(int32 WeaponGroup);

    UFUNCTION()
    AUR_Weapon* SelectWeaponG(int32 WeaponGroup);

    UFUNCTION()
    bool NextWeapon();

    UFUNCTION()
    bool PrevWeapon();

    UFUNCTION()
    void EquipWeapon(AUR_Weapon* InWeapon);

    UFUNCTION(Server, Reliable)
    void ServerEquipWeapon(AUR_Weapon* InWeapon);

    UFUNCTION()
    virtual void OnRep_InventoryW();

    UFUNCTION()
    virtual void OnRep_ActiveWeapon();

    UFUNCTION(BlueprintAuthorityOnly, BlueprintNativeEvent, BlueprintCallable)
    void Clear();
};
