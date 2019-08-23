// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Ammo_Rocket.h"

// Sets default values
AUR_Ammo_Rocket::AUR_Ammo_Rocket(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FObjectFinder<UStaticMesh> newAsset(TEXT("StaticMesh'/Game/SciFiWeapDark/Pickups/Darkness_RocketLauncher_Pickup.Darkness_RocketLauncher_Pickup'"));
	UStaticMesh* helper = newAsset.Object;
	AmmoMesh->SetStaticMesh(helper);
	AmmoName = "Rocket";
}