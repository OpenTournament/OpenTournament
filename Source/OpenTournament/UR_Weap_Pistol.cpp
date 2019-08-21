// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Weap_Pistol.h"

// Sets default values
AUR_Weap_Pistol::AUR_Weap_Pistol(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FObjectFinder<USkeletalMesh> newAsset(TEXT("SkeletalMesh'/Game/SciFiWeapDark/Weapons/Darkness_Pistol.Darkness_Pistol'"));
	USkeletalMesh* helper = newAsset.Object;
	Mesh1P->SetSkeletalMesh(helper);
	WeaponName = "Pistol";
}