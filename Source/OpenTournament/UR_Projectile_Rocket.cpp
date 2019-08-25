// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Projectile_Rocket.h"
#include "Engine.h"

// Sets default values
AUR_Projectile_Rocket::AUR_Projectile_Rocket(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FObjectFinder<UStaticMesh> newAsset(TEXT("StaticMesh'/Game/SciFiWeapDark/Weapons/Darkness_RocketLauncher_Ammo.Darkness_RocketLauncher_Ammo'"));
	UStaticMesh* helper = newAsset.Object;
	ProjMesh->SetStaticMesh(helper);

	Particles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Particles"));
	Particles->SetRelativeLocation(FVector::ZeroVector);
	Particles->AttachTo(RootComponent);

	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticlesInAssets(TEXT("ParticleSystem'/Game/SciFiWeapDark/FX/Particles/P_RocketLauncher_Trail_Dark.P_RocketLauncher_Trail_Dark'"));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticlesInAssetsExp(TEXT("ParticleSystem'/Game/SciFiWeapDark/FX/Particles/P_RocketLauncher_Explosion_Dark.P_RocketLauncher_Explosion_Dark'"));
	
	trail = ParticlesInAssets.Object;
	explosion = ParticlesInAssetsExp.Object;

	Particles->SetTemplate(trail);
}

// Called when the game starts or when spawned
void AUR_Projectile_Rocket::BeginPlay()
{
	Super::BeginPlay();
	CollisionComponent->SetGenerateOverlapEvents(true);
}

void AUR_Projectile_Rocket::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	Particles->SetTemplate(explosion);
	ProjMesh->DestroyComponent();
	DestroyAfter(2);
}





