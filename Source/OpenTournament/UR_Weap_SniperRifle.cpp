// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Weap_SniperRifle.h"

// Sets default values
AUR_Weap_SniperRifle::AUR_Weap_SniperRifle(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FObjectFinder<USkeletalMesh> newAsset(TEXT("SkeletalMesh'/Game/SciFiWeapDark/Weapons/Darkness_SniperRifle.Darkness_SniperRifle'"));
	USkeletalMesh* helper = newAsset.Object;
	Mesh1P->SetSkeletalMesh(helper);
	WeaponName = "Sniper Rifle";
	ProjectileClass = AUR_Projectile_Sniper::StaticClass();

	ConstructorHelpers::FObjectFinder<USoundCue> newAssetSound(TEXT("SoundCue'/Game/SciFiWeapDark/Sound/SniperRifle/SniperRifle_Lower_Cue.SniperRifle_Lower_Cue'"));
	USoundCue* helperSound;
	helperSound = newAssetSound.Object;
	Sound->SetSound(helperSound);

	AmmoName = "Sniper";
}

void AUR_Weap_SniperRifle::Fire(UWorld* World, FVector MuzzleLocation, FRotator MuzzleRotation, FActorSpawnParameters SpawnParams)
{
	if (ammoCount > 0) {
		AUR_Projectile_Sniper* Projectile = World->SpawnActor<AUR_Projectile_Sniper>(ProjectileClass, MuzzleLocation, MuzzleRotation, SpawnParams);
		if (Projectile)
		{
			FVector Direction = MuzzleRotation.Vector();
			Projectile->FireAt(Direction);
			ammoCount--;
		}
	}
	else
		GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, FString::Printf(TEXT("NO AMMO LEFT FOR %s!"), *WeaponName));

}