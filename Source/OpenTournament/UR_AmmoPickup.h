// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"

#include "UR_Pickup.h"

#include "UR_AmmoPickup.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AUR_Weapon;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
* Ammo Pickup Base Class
*/
UCLASS(Abstract, Blueprintable)
class OPENTOURNAMENT_API AUR_AmmoPickup : public AUR_Pickup
{
    GENERATED_BODY()
    
public:	

    AUR_AmmoPickup(const FObjectInitializer& ObjectInitializer);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    void OnPickup_Implementation(AUR_Character* PickupCharacter) override;
    
    /**
    * * @! TODO : This doesn't account for weapons that might have more than one
    *           type of ammunition (e.g. Bullets + Grenades, etc).
    *           This also doesn't account for weapons that might share ammo
    *           (e.g. Pistol Bullets / Minigun Bullets).
    */
    //   @! TODO        Should change to TSubclassOf<AUR_Ammo>. 
    /**
    * Weapon class this Ammo is Associated with.
    */
    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Ammo")
    TSubclassOf<AUR_Weapon> WeaponClass;

    /**
    * Count of Ammo Replenished by this Pickup
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ammo")
    int32 AmmoValue;
};
