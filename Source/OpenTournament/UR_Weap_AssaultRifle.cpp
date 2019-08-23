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
	
}

void AUR_Weap_AssaultRifle::Fire(UWorld* World, FVector MuzzleLocation, FRotator MuzzleRotation,FActorSpawnParameters SpawnParams)
{
	AUR_Projectile_Assault* Projectile = World->SpawnActor<AUR_Projectile_Assault>(ProjectileClass, MuzzleLocation, MuzzleRotation, SpawnParams);

	if (Projectile)
	{
		// Set the projectile's initial trajectory.
		FVector Direction = MuzzleRotation.Vector();
		Projectile->FireAt(Direction);
	}
}