// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Projectile_Grenade.h"

// Sets default values
AUR_Projectile_Grenade::AUR_Projectile_Grenade(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FObjectFinder<UStaticMesh> newAsset(TEXT("StaticMesh'/Game/SciFiWeapDark/Weapons/Darkness_GrenadeLauncher_Ammo.Darkness_GrenadeLauncher_Ammo'"));
	UStaticMesh* helper = newAsset.Object;
	ProjMesh->SetStaticMesh(helper);
}