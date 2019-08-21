// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Weap_AssaultRifle.h"

// Sets default values
AUR_Weap_AssaultRifle::AUR_Weap_AssaultRifle(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FObjectFinder<USkeletalMesh> newAsset(TEXT("SkeletalMesh'/Game/SciFiWeapDark/Weapons/Darkness_AssaultRifle.Darkness_AssaultRifle'"));
	USkeletalMesh* helper = newAsset.Object;
	Mesh1P->SetSkeletalMesh(helper);
	WeaponName = "Assault Rifle";
}