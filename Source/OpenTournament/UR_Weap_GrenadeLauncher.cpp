// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Weap_GrenadeLauncher.h"

// Sets default values
AUR_Weap_GrenadeLauncher::AUR_Weap_GrenadeLauncher(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FObjectFinder<USkeletalMesh> newAsset(TEXT("SkeletalMesh'/Game/SciFiWeapDark/Weapons/Darkness_GrenadeLauncher.Darkness_GrenadeLauncher'"));
	USkeletalMesh* helper = newAsset.Object;
	Mesh1P->SetSkeletalMesh(helper);
	WeaponName = "Grenade Launcher";
}