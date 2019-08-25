// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Projectile_Grenade.h"

// Sets default values
AUR_Projectile_Grenade::AUR_Projectile_Grenade(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FObjectFinder<UStaticMesh> newAsset(TEXT("StaticMesh'/Game/SciFiWeapDark/Weapons/Darkness_GrenadeLauncher_Ammo.Darkness_GrenadeLauncher_Ammo'"));
	UStaticMesh* helper = newAsset.Object;
	ProjMesh->SetStaticMesh(helper);

	Particles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Particles"));
	Particles->SetRelativeLocation(FVector::ZeroVector);
	Particles->AttachTo(RootComponent);


	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticlesInAssets(TEXT("ParticleSystem'/Game/SciFiWeapDark/FX/Particles/P_RocketLauncher_Trail_Light.P_RocketLauncher_Trail_Light'"));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticlesInAssetsExp(TEXT("ParticleSystem'/Game/SciFiWeapDark/FX/Particles/P_RocketLauncher_Explosion_light.P_RocketLauncher_Explosion_Light'"));
	
	trail = ParticlesInAssets.Object;
	explosion = ParticlesInAssetsExp.Object;

	Particles->SetTemplate(trail);
}


// Called when the game starts or when spawned
void AUR_Projectile_Grenade::BeginPlay()
{
	Super::BeginPlay();
	CollisionComponent->SetGenerateOverlapEvents(true);
	ProjectileMovementComponent->ProjectileGravityScale = 1;

}

void AUR_Projectile_Grenade::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	Particles->SetTemplate(explosion);
	ProjMesh->DestroyComponent();
	DestroyAfter(2);
}


