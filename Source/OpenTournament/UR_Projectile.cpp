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
    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
    CollisionComponent->BodyInstance.SetCollisionProfileName(TEXT("Projectile"));
    CollisionComponent->SetGenerateOverlapEvents(true);
    CollisionComponent->InitSphereRadius(15.0f);
    CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AUR_Projectile::Overlap);

    RootComponent = CollisionComponent;

    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
    ProjectileMovementComponent->SetUpdatedComponent(CollisionComponent);
    ProjectileMovementComponent->InitialSpeed = 5000.0f;
    ProjectileMovementComponent->MaxSpeed = 5000.0f;
    ProjectileMovementComponent->bRotationFollowsVelocity = true;
    ProjectileMovementComponent->ProjectileGravityScale = 0.f;
    ProjectileMovementComponent->bShouldBounce = false;

    StaticMeshComponent = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("StaticMeshComponent"));
    StaticMeshComponent->SetupAttachment(RootComponent);

    AudioComponent = ObjectInitializer.CreateDefaultSubobject<UAudioComponent>(this, TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Projectile::Overlap(UPrimitiveComponent * HitComp, AActor * Other, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
    // TODO
}

void AUR_Projectile::FireAt(const FVector& ShootDirection)
{
    ProjectileMovementComponent->Velocity = ShootDirection * ProjectileMovementComponent->InitialSpeed;
}

void AUR_Projectile::DestroyAfter(const int32 delay)
{
    SetActorEnableCollision(false);
    SetLifeSpan(2);
}