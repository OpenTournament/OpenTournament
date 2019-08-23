// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Ammo_Pistol.h"

// Sets default values
AUR_Ammo_Pistol::AUR_Ammo_Pistol(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FObjectFinder<UStaticMesh> newAsset(TEXT("StaticMesh'/Game/SciFiWeapDark/Pickups/Darkness_Pistol_Pickup.Darkness_Pistol_Pickup'"));
	UStaticMesh* helper = newAsset.Object;
	AmmoMesh->SetStaticMesh(helper);
	AmmoName = "Pistol";
}