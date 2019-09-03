// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Weap_GrenadeLauncher.h"

// Sets default values
AUR_Weap_GrenadeLauncher::AUR_Weap_GrenadeLauncher(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FObjectFinder<USkeletalMesh> newAsset(TEXT("SkeletalMesh'/Game/SciFiWeapDark/Weapons/Darkness_GrenadeLauncher.Darkness_GrenadeLauncher'"));
	USkeletalMesh* helper = newAsset.Object;
	Mesh1P->SetSkeletalMesh(helper);
	WeaponName = "Grenade Launcher";
	ProjectileClass = AUR_Projectile_Grenade::StaticClass();

	ConstructorHelpers::FObjectFinder<USoundCue> newAssetSound(TEXT("SoundCue'/Game/SciFiWeapDark/Sound/GrenadeLauncher/GrenadeLauncher_Lower_Cue.GrenadeLauncher_Lower_Cue'"));
	USoundCue* helperSound;
	helperSound = newAssetSound.Object;
	Sound->SetSound(helperSound);

	AmmoName = "Grenade";
}

void AUR_Weap_GrenadeLauncher::Fire(UWorld* World, FVector MuzzleLocation, FRotator MuzzleRotation, FActorSpawnParameters SpawnParams)
{
	if(ammoCount > 0){
		AUR_Projectile_Grenade* Projectile = World->SpawnActor<AUR_Projectile_Grenade>(ProjectileClass, MuzzleLocation, MuzzleRotation, SpawnParams);
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