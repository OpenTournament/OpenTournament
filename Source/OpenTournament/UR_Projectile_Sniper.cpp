// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Projectile_Sniper.h"

// Sets default values
AUR_Projectile_Sniper::AUR_Projectile_Sniper(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FObjectFinder<UStaticMesh> newAsset(TEXT("StaticMesh'/Game/FirstPersonProjectileMesh_4.FirstPersonProjectileMesh_4'"));
	UStaticMesh* helper = newAsset.Object;
	ProjMesh->SetStaticMesh(helper);
	ProjectileMovementComponent->InitialSpeed = ProjectileMovementComponent->InitialSpeed*8;
	ProjectileMovementComponent->MaxSpeed = ProjectileMovementComponent->InitialSpeed * 8;

}