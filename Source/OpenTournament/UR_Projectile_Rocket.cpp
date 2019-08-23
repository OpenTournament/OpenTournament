// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Projectile_Rocket.h"

// Sets default values
AUR_Projectile_Rocket::AUR_Projectile_Rocket(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FObjectFinder<UStaticMesh> newAsset(TEXT("StaticMesh'/Game/SciFiWeapDark/Weapons/Darkness_RocketLauncher_Ammo.Darkness_RocketLauncher_Ammo'"));
	UStaticMesh* helper = newAsset.Object;
	ProjMesh->SetStaticMesh(helper);
}