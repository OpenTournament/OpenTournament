// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

//TODO: move enum EWeaponState to a shared header
#include "UR_Weapon.h"

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

    UPROPERTY(ReplicatedUsing = OnRep_WeaponArray, BlueprintReadOnly, Category = "InventoryComponent")
    TArray<AUR_Weapon*> WeaponArray;

    /**
    * Ammo types are instanced and stored in this array, independently from weapons.
    * Ammo array is initially empty.
    * An ammo type is instanced and added here as soon as an ammo pack OR a weapon using this type is picked up.
    * Like weapons, there are no duplicates of the same class in the array.
    */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "InventoryComponent")
    TArray<AUR_Ammo*> AmmoArray;

    UPROPERTY(BlueprintReadOnly, Category = "InventoryComponent")
    AUR_Weapon* ActiveWeapon;

    /**
    * User current desired weapon.
    * It will become ActiveWeapon after the current weapon PutDown procedure is finished.
    */
    UPROPERTY(ReplicatedUsing = OnRep_DesiredWeapon, BlueprintReadOnly, Category = "InventoryComponent")
    AUR_Weapon* DesiredWeapon;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapon")
    bool IsLocallyControlled() const;

    UFUNCTION(BlueprintAuthorityOnly)
    virtual void AddWeapon(AUR_Weapon* InWeapon);

    UFUNCTION(BlueprintAuthorityOnly)
    virtual void AddAmmo(TSubclassOf<AUR_Ammo> InAmmoClass);

    UFUNCTION(BlueprintCallable)
    virtual AUR_Ammo* GetAmmoByClass(TSubclassOf<AUR_Ammo> InAmmoClass, bool bAllowCreate = false);

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
    void SetDesiredWeapon(AUR_Weapon* InWeapon);

    UFUNCTION(Server, Reliable)
    void ServerSetDesiredWeapon(AUR_Weapon* InWeapon);

    /**
    * Callback bound to ActiveWeapon's delegate.
    * Primarily used to wait for PutDown during swap procedures.
    */
    UFUNCTION()
    void OnActiveWeaponStateChanged(AUR_Weapon* Weapon, EWeaponState NewState);

    UFUNCTION()
    void SetActiveWeapon(AUR_Weapon* InWeapon);

    UFUNCTION(BlueprintAuthorityOnly, BlueprintNativeEvent, BlueprintCallable)
    void Clear();

protected:

    UFUNCTION()
    virtual void OnRep_WeaponArray();

    UFUNCTION()
    virtual void OnRep_DesiredWeapon();

};
