// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "UR_Ammo.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AUR_Weapon;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
* Describes a type of ammo, and acts as a container when instanced.
* Ammo types classes are referenced by ammo bases and weapons.
* Ammo types are instanced and stored in their own array in InventoryComponent, independently from weapons.
* Weapons can make use of multiple ammo types.
* An ammo type can be shared across multiple weapons.
*/
UCLASS(Blueprintable, NotPlaceable)
class OPENTOURNAMENT_API AUR_Ammo
    : public AActor
{
    GENERATED_BODY()

    AUR_Ammo();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
    /**
    * Ammo name.
    * Not sure if useful.
    */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText AmmoName;

    /**
    * Maximum ammo reachable by picking up ammo packs.
    * Should be >= WeaponPickupMaxAmmo.
    */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 MaxAmmo;

    /**
    * Current ammo amount.
    */
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AmmoCount)
    int32 AmmoCount;

public:
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
    virtual void StackAmmo(int32 InAmount, AUR_Weapon* FromWeapon = nullptr);

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
    virtual void ConsumeAmmo(int32 Amount = 1);

    /**
    * Set ammo count to desired value regardless of MaxAmmo.
    * Still hard-capped between 0 and 999.
    */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
    virtual void SetAmmoCount(int32 NewAmmoCount);

protected:
    UFUNCTION()
    virtual void OnRep_AmmoCount(int32 OldAmmoCount);
};
