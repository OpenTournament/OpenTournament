// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Projectile_Sniper.h"
#include "Engine.h"

// Sets default values
AUR_Projectile_Sniper::AUR_Projectile_Sniper(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FObjectFinder<UStaticMesh> newAsset(TEXT("StaticMesh'/Game/FirstPersonProjectileMesh_4.FirstPersonProjectileMesh_4'"));
	UStaticMesh* helper = newAsset.Object;
	ProjMesh->SetStaticMesh(helper);
	ProjectileMovementComponent->InitialSpeed = ProjectileMovementComponent->InitialSpeed*8;
	ProjectileMovementComponent->MaxSpeed = ProjectileMovementComponent->InitialSpeed * 8;

	Particles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Particles"));
	Particles->SetRelativeLocation(FVector::ZeroVector);
	Particles->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticlesInAssets(TEXT("ParticleSystem'/Game/SciFiWeapDark/FX/Particles/P_SniperRifle_Tracer_Light.P_SniperRifle_Tracer_Light'"));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticlesInAssets2(TEXT("ParticleSystem'/Game/SciFiWeapDark/FX/Particles/P_Impact_Stone_Medium_Light.P_Impact_Stone_Medium_Light'"));

	trail = ParticlesInAssets.Object;
	impact = ParticlesInAssets2.Object;

	Particles->SetTemplate(ParticlesInAssets.Object);

}

// Called when the game starts or when spawned
void AUR_Projectile_Sniper::BeginPlay()
{
	Super::BeginPlay();
	CollisionComponent->OnComponentHit.AddDynamic(this, &AUR_Projectile_Sniper::OnHit);
	CollisionComponent->SetGenerateOverlapEvents(true);
}

void AUR_Projectile_Sniper::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	Particles->SetTemplate(impact);
	OtherActor->TakeDamage(70, FDamageEvent::FDamageEvent(), NULL, this);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Damage Event by SNIPER RIFLE 1")));
	ProjMesh->DestroyComponent();
	DestroyAfter(3);
}