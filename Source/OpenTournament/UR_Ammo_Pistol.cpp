// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Ammo_Pistol.h"
#include "Engine.h"

// Sets default values
AUR_Ammo_Pistol::AUR_Ammo_Pistol(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FObjectFinder<UStaticMesh> newAsset(TEXT("StaticMesh'/Game/SciFiWeapDark/Pickups/Darkness_Pistol_Pickup.Darkness_Pistol_Pickup'"));
	UStaticMesh* helper = newAsset.Object;
	AmmoMesh->SetStaticMesh(helper);
	AmmoName = "Pistol";

	ConstructorHelpers::FObjectFinder<USoundCue> newAssetSound(TEXT("SoundCue'/Game/SciFiWeapDark/Sound/Pistol/Pistol_AmmoPickup_Cue.Pistol_AmmoPickup_Cue'"));
	USoundCue* helperSound;
	helperSound = newAssetSound.Object;
	Sound->SetSound(helperSound);

	amount = 20;

}