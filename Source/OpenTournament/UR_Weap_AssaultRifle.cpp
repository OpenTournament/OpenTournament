// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Weap_AssaultRifle.h"

// Sets default values
AUR_Weap_AssaultRifle::AUR_Weap_AssaultRifle(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FObjectFinder<USkeletalMesh> newAsset(TEXT("SkeletalMesh'/Game/SciFiWeapDark/Weapons/Darkness_AssaultRifle.Darkness_AssaultRifle'"));
	USkeletalMesh* helper = newAsset.Object;
	Mesh1P->SetSkeletalMesh(helper);
	WeaponName = "Assault Rifle";
	ProjectileClass = AUR_Projectile_Assault::StaticClass();

	ConstructorHelpers::FObjectFinder<USoundCue> newAssetSound(TEXT("SoundCue'/Game/SciFiWeapDark/Sound/Rifle/Rifle_Lower_Cue.Rifle_Lower_Cue'"));
	USoundCue* helperSound;
	helperSound = newAssetSound.Object;
	Sound->SetSound(helperSound);

	/*ConstructorHelpers::FObjectFinder<USoundCue> newAssetSoundFire(TEXT("SoundCue'/Game/SciFiWeapDark/Sound/Rifle/RifleA_Fire_Cue.RifleA_Fire_Cue'"));
	USoundCue* helperSoundFire;
	helperSoundFire = newAssetSoundFire.Object;
	SoundFire->SetSound(helperSoundFire);*/
}

void AUR_Weap_AssaultRifle::Fire(UWorld* World, FVector MuzzleLocation, FRotator MuzzleRotation,FActorSpawnParameters SpawnParams)
{
	AUR_Projectile_Assault* Projectile = World->SpawnActor<AUR_Projectile_Assault>(ProjectileClass, MuzzleLocation, MuzzleRotation, SpawnParams);

	if (Projectile)
	{
		/*SoundFire->SetActive(true);
		SoundFire = UGameplayStatics::SpawnSoundAtLocation(this, Sound->Sound, this->GetActorLocation(), FRotator::ZeroRotator, 1.0f, 1.0f, 0.0f, nullptr, nullptr, true);*/
		// Set the projectile's initial trajectory.
		FVector Direction = MuzzleRotation.Vector();
		Projectile->FireAt(Direction);
	}
}