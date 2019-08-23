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
}

void AUR_Weap_GrenadeLauncher::Fire(UWorld* World, FVector MuzzleLocation, FRotator MuzzleRotation, FActorSpawnParameters SpawnParams)
{
	AUR_Projectile_Grenade* Projectile = World->SpawnActor<AUR_Projectile_Grenade>(ProjectileClass, MuzzleLocation, MuzzleRotation, SpawnParams);
	if (Projectile)
	{
		// Set the projectile's initial trajectory.
		FVector Direction = MuzzleRotation.Vector();
		Projectile->FireAt(Direction);
	}
}