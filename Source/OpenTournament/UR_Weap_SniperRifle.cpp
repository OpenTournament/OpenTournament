// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Weap_SniperRifle.h"

// Sets default values
AUR_Weap_SniperRifle::AUR_Weap_SniperRifle(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FObjectFinder<USkeletalMesh> newAsset(TEXT("SkeletalMesh'/Game/SciFiWeapDark/Weapons/Darkness_SniperRifle.Darkness_SniperRifle'"));
	USkeletalMesh* helper = newAsset.Object;
	Mesh1P->SetSkeletalMesh(helper);
	WeaponName = "Sniper Rifle";
	ProjectileClass = AUR_Projectile::StaticClass();
}

void AUR_Weap_SniperRifle::Fire(UWorld* World, FVector MuzzleLocation, FRotator MuzzleRotation, FActorSpawnParameters SpawnParams)
{
	AUR_Projectile* Projectile = World->SpawnActor<AUR_Projectile>(ProjectileClass, MuzzleLocation, MuzzleRotation, SpawnParams);
	if (Projectile)
	{
		// Set the projectile's initial trajectory.
		FVector Direction = MuzzleRotation.Vector();
		Projectile->FireAt(Direction);
	}
}