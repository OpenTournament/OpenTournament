// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Weap_RocketLauncher.h"

// Sets default values
AUR_Weap_RocketLauncher::AUR_Weap_RocketLauncher(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FObjectFinder<USkeletalMesh> newAsset(TEXT("SkeletalMesh'/Game/SciFiWeapDark/Weapons/Darkness_RocketLauncher.Darkness_RocketLauncher'"));
	USkeletalMesh *helper = newAsset.Object;
	Mesh1P->SetSkeletalMesh(helper);
	WeaponName = "Rocket Launcher";
}