// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Projectile_Rocket.h"
#include "Engine.h"

// Sets default values
AUR_Projectile_Rocket::AUR_Projectile_Rocket(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FObjectFinder<UStaticMesh> newAsset(TEXT("StaticMesh'/Game/SciFiWeapDark/Weapons/Darkness_RocketLauncher_Ammo.Darkness_RocketLauncher_Ammo'"));
	UStaticMesh* helper = newAsset.Object;
	ProjMesh->SetStaticMesh(helper);

	ExplosionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("ExplosionComponent"));
	ExplosionComponent->BodyInstance.SetCollisionProfileName(TEXT("Explosion"));
	ExplosionComponent->InitSphereRadius(600.0f);

	ExplosionComponent->SetupAttachment(RootComponent);

	Particles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Particles"));
	Particles->SetRelativeLocation(FVector::ZeroVector);
	Particles->SetupAttachment(RootComponent);

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
	CollisionComponent->OnComponentHit.AddDynamic(this, &AUR_Projectile_Rocket::OnHit);
	ExplosionComponent->SetGenerateOverlapEvents(true);
	CollisionComponent->SetGenerateOverlapEvents(true);
}

void AUR_Projectile_Rocket::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	Particles->SetTemplate(explosion);
	OtherActor->TakeDamage(100, FDamageEvent::FDamageEvent() , NULL, this);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Damage Event ROCKET LAUNCHER")));
	if (ExplosionComponent->IsOverlappingActor(OtherActor))
		DamageNearActors();
	ProjMesh->DestroyComponent();
	DestroyAfter(3);


}

void AUR_Projectile_Rocket::DamageNearActors() {
	TArray<AActor*> actors;
	float distance_centers;
	ExplosionComponent->GetOverlappingActors(actors, TSubclassOf<AUR_Character>());
	for (auto &actor : actors) {
		FVector a = ExplosionComponent->GetComponentLocation();
		FVector b = actor->GetActorLocation();
		distance_centers = FVector::Distance(a, b);
		float aoeDamageValue = distance_centers / 5;
		actor->TakeDamage(distance_centers / 20, FDamageEvent::FDamageEvent(), NULL, this);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Damage Event AOE on Actor - DAMAGE %f with distance %f"), aoeDamageValue, distance_centers));
	}
}





