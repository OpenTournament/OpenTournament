// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Weap_Pistol.h"

// Sets default values
AUR_Weap_Pistol::AUR_Weap_Pistol(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FObjectFinder<USkeletalMesh> newAsset(TEXT("SkeletalMesh'/Game/SciFiWeapDark/Weapons/Darkness_Pistol.Darkness_Pistol'"));
	USkeletalMesh* helper = newAsset.Object;
	Mesh1P->SetSkeletalMesh(helper);
	WeaponName = "Pistol";
	ProjectileClass = AUR_Projectile_Assault::StaticClass();

	ConstructorHelpers::FObjectFinder<USoundCue> newAssetSound(TEXT("SoundCue'/Game/SciFiWeapDark/Sound/Pistol/Pistol_Lower_Cue.Pistol_Lower_Cue'"));
	USoundCue* helperSound;
	helperSound = newAssetSound.Object;
	Sound->SetSound(helperSound);

	AmmoName = "Pistol";
}

void AUR_Weap_Pistol::Fire(UWorld* World, FVector MuzzleLocation, FRotator MuzzleRotation, FActorSpawnParameters SpawnParams)
{
	if (ammoCount > 0) {
		AUR_Projectile_Assault* Projectile = World->SpawnActor<AUR_Projectile_Assault>(ProjectileClass, MuzzleLocation, MuzzleRotation, SpawnParams);
		if (Projectile)
		{
			FVector Direction = MuzzleRotation.Vector();
			Projectile->FireAt(Direction);
			ammoCount;
		}
	}
	else
		GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, FString::Printf(TEXT("NO AMMO LEFT FOR %s!"), *WeaponName));
}