// Fill out your copyright notice in the Description page of Project Settings.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Projectile.h"

#include "ConstructorHelpers.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystemComponent.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

// Sets default values
AUR_Projectile::AUR_Projectile(const FObjectInitializer& ObjectInitializer)
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
    CollisionComponent->BodyInstance.SetCollisionProfileName(TEXT("Projectile"));
    CollisionComponent->InitSphereRadius(15.0f);
    RootComponent = CollisionComponent;

    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
    ProjectileMovementComponent->SetUpdatedComponent(CollisionComponent);
    ProjectileMovementComponent->InitialSpeed = 5000.0f;
    ProjectileMovementComponent->MaxSpeed = 5000.0f;
    ProjectileMovementComponent->bRotationFollowsVelocity = true;
    ProjectileMovementComponent->bShouldBounce = true;

    SM_TBox = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Box Mesh"));
    SM_TBox->SetupAttachment(RootComponent);

    ProjMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("ProjMesh1"));
    ProjMesh->SetupAttachment(RootComponent);

    SoundFire = ObjectInitializer.CreateDefaultSubobject<UAudioComponent>(this, TEXT("SoundFire"));
    SoundFire->SetupAttachment(RootComponent);

    SoundHit = ObjectInitializer.CreateDefaultSubobject<UAudioComponent>(this, TEXT("SoundHit"));
    SoundHit->SetupAttachment(RootComponent);

    ConstructorHelpers::FObjectFinder<UStaticMesh> newAsset(TEXT("StaticMesh'/Game/FirstPerson/Meshes/FirstPersonProjectileMesh.FirstPersonProjectileMesh'"));
    UStaticMesh* helper = newAsset.Object;
    ProjMesh->SetStaticMesh(helper);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

// Called when the game starts or when spawned
void AUR_Projectile::BeginPlay()
{
    Super::BeginPlay();
    ProjectileMovementComponent->ProjectileGravityScale = 0;
    CollisionComponent->SetGenerateOverlapEvents(true);
}

// Called every frame
void AUR_Projectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AUR_Projectile::FireAt(const FVector& ShootDirection)
{
    ProjectileMovementComponent->Velocity = ShootDirection * ProjectileMovementComponent->InitialSpeed;
}

void AUR_Projectile::DestroyAfter(int delay)
{
    SetActorEnableCollision(false);
    SetLifeSpan(2);
}

