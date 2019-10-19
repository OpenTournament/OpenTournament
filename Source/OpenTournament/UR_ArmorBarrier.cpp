// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_ArmorBarrier.h"

// Sets default values
AUR_ArmorBarrier::AUR_ArmorBarrier(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FObjectFinder<UStaticMesh> newAsset(TEXT("StaticMesh'/Game/ArmorBarrierMesh.ArmorBarrierMesh'"));
	UStaticMesh* helper = newAsset.Object;
	ArmorMesh->SetStaticMesh(helper);

	ConstructorHelpers::FObjectFinder<USoundCue> newAssetSound(TEXT("SoundCue'/Game/SciFiWeapDark/Sound/SniperRifle/SniperRifle_AmmoPickup_Cue.SniperRifle_AmmoPickup_Cue'")); //TODO : Replace
	USoundCue* helperSound;
	helperSound = newAssetSound.Object;
	Sound->SetSound(helperSound);

	armorVal = 199;
	armorBarrier = true;
}