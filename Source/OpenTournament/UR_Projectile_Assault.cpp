// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Projectile_Assault.h"
#include "Engine.h"

// Sets default values
AUR_Projectile_Assault::AUR_Projectile_Assault(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FObjectFinder<UStaticMesh> newAsset(TEXT("StaticMesh'/Game/SciFiWeapDark/Weapons/Darkness_AssaultRifle_Ammo.Darkness_AssaultRifle_Ammo'"));
	UStaticMesh* helper = newAsset.Object;
	ProjMesh->SetStaticMesh(helper);

	Particles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Particles"));
	Particles->SetRelativeLocation(FVector::ZeroVector);
	Particles->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticlesInAssets(TEXT("ParticleSystem'/Game/SciFiWeapDark/FX/Particles/P_AssaultRifle_Tracer_Dark.P_AssaultRifle_Tracer_Dark'"));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticlesInAssets2(TEXT("ParticleSystem'/Game/SciFiWeapDark/FX/Particles/P_Impact_Wood_Large_Dark.P_Impact_Wood_Large_Dark'"));

	trail = ParticlesInAssets.Object;
	impact = ParticlesInAssets2.Object;


	Particles->SetTemplate(ParticlesInAssets.Object);
}

// Called when the game starts or when spawned
void AUR_Projectile_Assault::BeginPlay()
{
	Super::BeginPlay();
	CollisionComponent->OnComponentHit.AddDynamic(this, &AUR_Projectile_Assault::OnHit);
	CollisionComponent->SetGenerateOverlapEvents(true);
}

void AUR_Projectile_Assault::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	Particles->SetTemplate(impact);
	OtherActor->TakeDamage(5, FDamageEvent::FDamageEvent(), NULL, this);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Damage Event by ASSAULT RIFLE 1")));
	ProjMesh->DestroyComponent();
	DestroyAfter(3);
}
