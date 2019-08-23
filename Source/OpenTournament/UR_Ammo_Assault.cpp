// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Ammo_Assault.h"

// Sets default values
AUR_Ammo_Assault::AUR_Ammo_Assault(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FObjectFinder<UStaticMesh> newAsset(TEXT("StaticMesh'/Game/SciFiWeapDark/Pickups/Darkness_AssaultRifle_Pickup.Darkness_AssaultRifle_Pickup'"));
	UStaticMesh* helper = newAsset.Object;
	AmmoMesh->SetStaticMesh(helper);
	AmmoName = "Assault";
}