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


	/*ConstructorHelpers::FObjectFinder<USoundCue> newAssetSoundFire(TEXT("SoundCue'/Game/SciFiWeapDark/Sound/Pistol/PistolA_Fire_Cue.PistolA_Fire_Cue'"));
	USoundCue* helperSoundFire;
	helperSoundFire = newAssetSoundFire.Object;
	SoundFire->SetSound(helperSoundFire);
	
		SoundCue'/Game/SciFiWeapDark/Sound/Pistol/PistolA_Fire_Cue.PistolA_Fire_Cue'
		SoundCue'/Game/SciFiWeapDark/Sound/Pistol/Pistol_Whip_Cue.Pistol_Whip_Cue'*/
}

void AUR_Weap_Pistol::Fire(UWorld* World, FVector MuzzleLocation, FRotator MuzzleRotation, FActorSpawnParameters SpawnParams)
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