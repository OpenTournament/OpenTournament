// Copyright 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_InventoryComponent.h"

#include "UnrealNetwork.h"
#include "Engine.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_InventoryComponent::UUR_InventoryComponent()
{
	bReplicates = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_InventoryComponent::Add(AUR_Weapon* weapon)
{
	if (!InventoryW.Contains(weapon)) {
		InventoryW.Add(weapon);
		GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, FString::Printf(TEXT("You picked the %s"), *weapon->WeaponName));
		GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Blue, FString::Printf(TEXT("Weapon ammo count: %d"), weapon->ammoCount));
		AmmoCountInInventory(weapon);
	}
	else 
		GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Blue, FString::Printf(TEXT("You already have the %s"), *weapon->WeaponName));
}

void UUR_InventoryComponent::Add(AUR_Ammo* ammo)
{
	/*if (InventoryA.Contains(ammo)) {
		for (auto& ammo2 : InventoryA)
		{
			if (ammo2->AmmoName == *ammo->AmmoName) {
				ammo2->amount += ammo->amount;
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, FString::Printf(TEXT("Added %s ammo. Current amount: %d"), *ammo2->AmmoName, ammo2->amount));
			}
		}
	}
	else {*/
		InventoryA.Add(ammo);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("You picked the %s"), *ammo->AmmoName));
	//}
		UpdateWeaponAmmo(ammo);
}

void UUR_InventoryComponent::AmmoCountInInventory(AUR_Weapon* weapon) 
{
	for (auto& ammo : InventoryA)
	{
		if (weapon->AmmoName == *ammo->AmmoName) {
			weapon->ammoCount += ammo->amount;
		}
	}
}

void UUR_InventoryComponent::UpdateWeaponAmmo(AUR_Ammo* ammo) 
{
	for (auto& weapon : InventoryW)
	{
		if (weapon->AmmoName == *ammo->AmmoName) {
			weapon->ammoCount += ammo->amount;
		}
	}
}


void UUR_InventoryComponent::ShowInventory()
{
	for (auto& weapon : InventoryW)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Weapons in inventory: %s with Ammo Count: %d"), *weapon->WeaponName, weapon->ammoCount));
	}

	for (auto& ammo : InventoryA)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Ammo in inventory: %s"), *ammo->AmmoName));
	}

}

int32 UUR_InventoryComponent::SelectWeapon(int number)
{
	FString name = "null weapon";
	int32 result = 0;
		for (auto& weapon : InventoryW)
		{
			if (number == 1 && weapon->WeaponName == "Assault Rifle") 
			{ 
				result = 1; 
				name = weapon->WeaponName;
				weapon->AttachMeshToPawn();
				ActiveWeapon = weapon;
				weapon->setEquipped(true);
			}
			else if (number == 2 && weapon->WeaponName == "Shotgun")
			{
				result = 2;
				name = weapon->WeaponName;
				weapon->AttachMeshToPawn();
				ActiveWeapon = weapon;
				weapon->setEquipped(true);
			}
			else if (number == 3 && weapon->WeaponName == "Rocket Launcher")
			{
				result = 3;
				name = weapon->WeaponName;
				weapon->AttachMeshToPawn();
				ActiveWeapon = weapon;
				weapon->setEquipped(true);
			}
			else if (number == 4 && weapon->WeaponName == "Grenade Launcher")
			{
				result = 4;
				name = weapon->WeaponName;
				weapon->AttachMeshToPawn();
				ActiveWeapon = weapon;
				weapon->setEquipped(true);
			}
			else if (number == 5 && weapon->WeaponName == "Sniper Rifle")
			{
				result = 5;
				name = weapon->WeaponName;
				weapon->AttachMeshToPawn();
				ActiveWeapon = weapon;
				weapon->setEquipped(true);
			}
			else if (number == 0 && weapon->WeaponName == "Pistol")
			{
				result = 0;
				name = weapon->WeaponName;
				weapon->AttachMeshToPawn();
				ActiveWeapon = weapon;
				weapon->setEquipped(true);
			}
			else {
				weapon->DetachMeshFromPawn();
				weapon->setEquipped(false);
			}
		}
	
	GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Yellow, FString::Printf(TEXT("Weapon name -> %s \n"), *name));
	return result;
}

AUR_Weapon * UUR_InventoryComponent::SelectWeaponG(int number)
{

	FString name = "null weapon";
	int32 result = 0;

	for (auto& weapon : InventoryW)
	{
		if (number == 1 && weapon->WeaponName == "Assault Rifle")
		{
			result = 1;
			name = weapon->WeaponName;
			weapon->AttachMeshToPawn();
			ActiveWeapon = weapon;
			weapon->setEquipped(true);
		}
		else if (number == 2 && weapon->WeaponName == "Shotgun")
		{
			result = 2;
			name = weapon->WeaponName;
			weapon->AttachMeshToPawn();
			ActiveWeapon = weapon;
			weapon->setEquipped(true);
		}
		else if (number == 3 && weapon->WeaponName == "Rocket Launcher")
		{
			result = 3;
			name = weapon->WeaponName;
			weapon->AttachMeshToPawn();
			ActiveWeapon = weapon;
			weapon->setEquipped(true);
		}
		else if (number == 4 && weapon->WeaponName == "Grenade Launcher")
		{
			result = 4;
			name = weapon->WeaponName;
			weapon->AttachMeshToPawn();
			ActiveWeapon = weapon;
			weapon->setEquipped(true);
		}
		else if (number == 5 && weapon->WeaponName == "Sniper Rifle")
		{
			result = 5;
			name = weapon->WeaponName;
			weapon->AttachMeshToPawn();
			ActiveWeapon = weapon;
			weapon->setEquipped(true);
		}
		else if (number == 0 && weapon->WeaponName == "Pistol")
		{
			result = 0;
			name = weapon->WeaponName;
			weapon->AttachMeshToPawn();
			ActiveWeapon = weapon;
			weapon->setEquipped(true);
		}
		else {
			weapon->DetachMeshFromPawn();
			weapon->setEquipped(false);
		}
	}

	GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Yellow, FString::Printf(TEXT("Weapon name -> %s \n"), *name));
	return ActiveWeapon;
}