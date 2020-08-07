// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Ammo.h"

#include "Net/UnrealNetwork.h"

#include "UR_Character.h"
#include "UR_InventoryComponent.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_Ammo::AUR_Ammo()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	bOnlyRelevantToOwner = true;
	SetReplicatingMovement(false);

	AmmoName = TEXT("Ammo");
	AmmoPerWeapon = 10;
	WeaponPickupMaxAmmo = 30;
	AmmoPerPickup = 10;
	MaxAmmo = 30;

	AmmoCount = 0;
	bPickedUpFirstWeapon = false;
}

void AUR_Ammo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AUR_Ammo, AmmoCount, COND_OwnerOnly);
}

void AUR_Ammo::OnRep_AmmoCount(int32 OldAmmoCount)
{
	if (AmmoCount != OldAmmoCount)
	{
		if (AUR_Character* Char = Cast<AUR_Character>(GetOwner()))
		{
			if (Char->InventoryComponent && Char->InventoryComponent->ActiveWeapon && Char->InventoryComponent->ActiveWeapon->AmmoClasses.Contains(GetClass()))
			{
				Char->InventoryComponent->ActiveWeapon->NotifyAmmoUpdated(this);
			}
		}
	}
}
