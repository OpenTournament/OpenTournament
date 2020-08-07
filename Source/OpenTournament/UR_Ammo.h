// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "UR_Ammo.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////////

/**
* Describes a type of ammo, and acts as a container when instanced.
* Ammo types classes are referenced by ammo bases and weapons.
* Ammo types are instanced and stored in their own array in InventoryComponent, independently from weapons.
* Weapons can make use of multiple ammo types.
* An ammo type can be shared across multiple weapons.
*/
UCLASS(Blueprintable, NotPlaceable)
class OPENTOURNAMENT_API AUR_Ammo : public AActor
{
	GENERATED_BODY()

	AUR_Ammo();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:

	/**
	* Ammo name.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString AmmoName;

	/**
	* Ammo given when a weapon using this ammo is picked up.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 AmmoPerWeapon;

	/**
	* Maximum ammo that can be stacked from picking up weapon multiple times.
	* Should be <= MaxAmmo.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 WeaponPickupMaxAmmo;

	/**
	* Ammo given when picking up an ammo pack of this type.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 AmmoPerPickup;

	/**
	* Maximum ammo reachable by picking up ammo packs.
	* Should be >= WeaponPickupMaxAmmo.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxAmmo;

	/**
	* Ammo pickup mesh.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMesh* PickupMesh;

	/**
	* Ammo pickup sound.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundBase* PickupSound;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AmmoCount)
	int32 AmmoCount;

	UPROPERTY()
	bool bPickedUpFirstWeapon;

public:

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	virtual void StackWeapon()
	{
		if (bPickedUpFirstWeapon)
		{
			if (AmmoCount < WeaponPickupMaxAmmo)
			{
				AmmoCount = FMath::Min(AmmoCount + AmmoPerWeapon, WeaponPickupMaxAmmo);
			}
		}
		else if (AmmoCount < MaxAmmo)
		{
			// No weapon picked up yet, assume all current stack is from ammo packs.
			// In this case we should not be limited by WeaponPickupMaxAmmo.
			AmmoCount = FMath::Min(AmmoCount + AmmoPerWeapon, MaxAmmo);
			bPickedUpFirstWeapon = true;
		}
	}

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	virtual void StackAmmoPack()
	{
		if (AmmoCount < MaxAmmo)
		{
			AmmoCount = FMath::Min(AmmoCount + AmmoPerPickup, MaxAmmo);
		}
	}

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	virtual void ConsumeAmmo(int32 Amount = 1)
	{
		int32 OldAmmoCount = AmmoCount;
		AmmoCount = FMath::Clamp(AmmoCount - Amount, 0, 999);
		OnRep_AmmoCount(OldAmmoCount);
	}

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	virtual void SetAmmoCount(int32 NewAmmoCount)
	{
		int32 OldAmmoCount = AmmoCount;
		AmmoCount = FMath::Clamp(NewAmmoCount, 0, 999);
		OnRep_AmmoCount(OldAmmoCount);
	}

protected:

	UFUNCTION()
	virtual void OnRep_AmmoCount(int32 OldAmmoCount);

};
