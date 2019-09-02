// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Projectile_Shotgun.h"
#include "Engine.h"

// Sets default values
AUR_Projectile_Shotgun::AUR_Projectile_Shotgun(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FObjectFinder<UStaticMesh> newAsset(TEXT("StaticMesh'/Game/SciFiWeapDark/Weapons/Darkness_Shotgun_Ammo.Darkness_Shotgun_Ammo'"));
	UStaticMesh* helper = newAsset.Object;
	ProjMesh->SetStaticMesh(helper);

	Particles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Particles"));
	Particles->SetRelativeLocation(FVector::ZeroVector);
	Particles->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticlesInAssets(TEXT("ParticleSystem'/Game/SciFiWeapDark/FX/Particles/P_Shotgun_Tracer_Light.P_Shotgun_Tracer_Light'"));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticlesInAssets2(TEXT("ParticleSystem'/Game/SciFiWeapDark/FX/Particles/P_Impact_Wood_Large_Light.P_Impact_Wood_Large_Light'"));

	ConstructorHelpers::FObjectFinder<USoundCue> newAssetSoundFire(TEXT("SoundCue'/Game/SciFiWeapDark/Sound/Shotgun/ShotgunA_Fire_Cue.ShotgunA_Fire_Cue'"));
	ConstructorHelpers::FObjectFinder<USoundCue> newAssetSoundHit(TEXT("SoundCue'/Game/SciFiWeapDark/Sound/Shotgun/Shotgun_ImpactBody_Cue.Shotgun_ImpactBody_Cue'"));



	USoundCue* helperSoundFire;
	helperSoundFire = newAssetSoundFire.Object;
	SoundFire->SetSound(helperSoundFire);

	USoundCue* helperSoundHit;
	helperSoundHit = newAssetSoundHit.Object;
	SoundHit->SetSound(helperSoundHit);

	trail = ParticlesInAssets.Object;
	impact = ParticlesInAssets2.Object;

	Particles->SetTemplate(ParticlesInAssets.Object);
}

// Called when the game starts or when spawned
void AUR_Projectile_Shotgun::BeginPlay()
{
	Super::BeginPlay();
	CollisionComponent->OnComponentHit.AddDynamic(this, &AUR_Projectile_Shotgun::OnHit);
	CollisionComponent->SetGenerateOverlapEvents(true);

	SoundFire->SetActive(true);
	SoundHit->SetActive(false);
	SoundFire = UGameplayStatics::SpawnSoundAtLocation(this, SoundFire->Sound, this->GetActorLocation(), FRotator::ZeroRotator, 1.0f, 1.0f, 0.0f, nullptr, nullptr, true);
}

void AUR_Projectile_Shotgun::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	SoundHit->SetActive(true);
	SoundFire = UGameplayStatics::SpawnSoundAtLocation(this, SoundHit->Sound, this->GetActorLocation(), FRotator::ZeroRotator, 1.0f, 1.0f, 0.0f, nullptr, nullptr, true);
	Particles->SetTemplate(impact);
	OtherActor->TakeDamage(50, FDamageEvent::FDamageEvent(), NULL, this);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Damage Event by ASSAULT RIFLE 1")));
	ProjMesh->DestroyComponent();
	DestroyAfter(3);
}