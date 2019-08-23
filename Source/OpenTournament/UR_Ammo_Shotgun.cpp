// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Ammo_Shotgun.h"

// Sets default values
AUR_Ammo_Shotgun::AUR_Ammo_Shotgun(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FObjectFinder<UStaticMesh> newAsset(TEXT("StaticMesh'/Game/SciFiWeapDark/Pickups/Darkness_Shotgun_Pickup.Darkness_Shotgun_Pickup'"));
	UStaticMesh* helper = newAsset.Object;
	AmmoMesh->SetStaticMesh(helper);
	AmmoName = "Shotgun";
}