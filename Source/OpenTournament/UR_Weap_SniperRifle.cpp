// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Weap_SniperRifle.h"

// Sets default values
AUR_Weap_SniperRifle::AUR_Weap_SniperRifle(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FObjectFinder<USkeletalMesh> newAsset(TEXT("SkeletalMesh'/Game/SciFiWeapDark/Weapons/Darkness_SniperRifle.Darkness_SniperRifle'"));
	USkeletalMesh* helper = newAsset.Object;
	Mesh1P->SetSkeletalMesh(helper);
	WeaponName = "Sniper Rifle";
}