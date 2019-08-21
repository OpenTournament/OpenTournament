// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Weap_Shotgun.h"

// Sets default values
AUR_Weap_Shotgun::AUR_Weap_Shotgun(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FObjectFinder<USkeletalMesh> newAsset(TEXT("SkeletalMesh'/Game/SciFiWeapDark/Weapons/Darkness_Shotgun.Darkness_Shotgun'"));
	USkeletalMesh* helper = newAsset.Object;
	Mesh1P->SetSkeletalMesh(helper);
	WeaponName = "Shotgun";
}