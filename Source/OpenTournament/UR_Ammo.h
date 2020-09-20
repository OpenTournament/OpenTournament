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
	* Not sure if useful.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString AmmoName;

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
	virtual void StackAmmo(int32 InAmount, AUR_Weapon* FromWeapon = NULL)
	{
		int32 AmmoCap = MaxAmmo;

		//NOTE: Here we can add something to handle quake-like case, to prevent weapon ammo stacking.
		// I don't want to use properties for this because it's probably gonna end up depending on gamemode.
		// For example casual FFA would have fast respawn times ==> prevent weapon ammo stacking
		// But Duel have long respawn times ==> allow weapon ammo stacking
		// So the gamemode should be responsible for controlling this behavior.

		// We can add a hook here which does something like this :
		/*
		if (InventoryComponent->WeaponArray.Contains(FromWeapon))
		{
			int32 WeaponAmmo = 0;
			for (const auto& AmmoDef : FromWeapon->AmmoDefinitions)
			{
				if (AmmoDef.AmmoClass == StaticClass() && AmmoDef.AmmoAmount > WeaponAmmo)
				{
					WeaponAmmo = AmmoDef.AmmoAmount;
				}
			}
			if (WeaponAmmo > 0)
			{
				AmmoCap = WeaponAmmo + 1;
			}
		}
		*/

		if (AmmoCount < AmmoCap)
		{
			AmmoCount = FMath::Min(AmmoCount + InAmount, AmmoCap);
		}
	}

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	virtual void ConsumeAmmo(int32 Amount = 1)
	{
		int32 OldAmmoCount = AmmoCount;
		AmmoCount = FMath::Clamp(AmmoCount - Amount, 0, 999);
		OnRep_AmmoCount(OldAmmoCount);
	}

	/**
	* Set ammo count to desired value regardless of MaxAmmo.
	*/
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
