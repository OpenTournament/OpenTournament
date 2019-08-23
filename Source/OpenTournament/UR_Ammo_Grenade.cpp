// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Ammo_Grenade.h"

// Sets default values
AUR_Ammo_Grenade::AUR_Ammo_Grenade(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FObjectFinder<UStaticMesh> newAsset(TEXT("StaticMesh'/Game/SciFiWeapDark/Pickups/Darkness_GrenadeLauncher_Pickup.Darkness_GrenadeLauncher_Pickup'"));
	UStaticMesh* helper = newAsset.Object;
	AmmoMesh->SetStaticMesh(helper);
	AmmoName = "Grenade";
}