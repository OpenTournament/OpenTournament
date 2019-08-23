// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Projectile_Shotgun.h"

// Sets default values
AUR_Projectile_Shotgun::AUR_Projectile_Shotgun(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FObjectFinder<UStaticMesh> newAsset(TEXT("StaticMesh'/Game/SciFiWeapDark/Weapons/Darkness_Shotgun_Ammo.Darkness_Shotgun_Ammo'"));
	UStaticMesh* helper = newAsset.Object;
	ProjMesh->SetStaticMesh(helper);

	Particles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Particles"));
	Particles->SetRelativeLocation(FVector::ZeroVector);
	Particles->AttachTo(RootComponent);

	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticlesInAssets(TEXT("ParticleSystem'/Game/SciFiWeapDark/FX/Particles/P_AssaultRifle_Tracer_Dark.P_AssaultRifle_Tracer_Dark'"));

	Particles->SetTemplate(ParticlesInAssets.Object);
}