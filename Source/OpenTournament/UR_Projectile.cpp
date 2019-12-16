// Fill out your copyright notice in the Description page of Project Settings.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Projectile.h"

#include "ConstructorHelpers.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystemComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

// Sets default values
AUR_Projectile::AUR_Projectile(const FObjectInitializer& ObjectInitializer)
{
    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
    CollisionComponent->BodyInstance.SetCollisionProfileName(TEXT("Projectile"));
    CollisionComponent->SetGenerateOverlapEvents(true);
    CollisionComponent->InitSphereRadius(15.0f);
    CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AUR_Projectile::Overlap);
    CollisionComponent->OnComponentHit.AddDynamic(this, &AUR_Projectile::OnHit);

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

    Particles = ObjectInitializer.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("Particles"));
    Particles->SetupAttachment(RootComponent);

    bReplicates = true;
    bNetTemporary = true;

    BaseDamage = 100.f;
    SplashRadius = 0.0f;
    InnerSplashRadius = 10.f;
    SplashMinimumDamage = 1.0f;
    SplashFalloff = 1.0f;
    DamageTypeClass = UDamageType::StaticClass();
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Projectile::BeginPlay()
{
    Super::BeginPlay();

    // Avoid colliding with shooter
    if (GetInstigator())
    {
        CollisionComponent->IgnoreActorWhenMoving(GetInstigator(), true);
        GetInstigator()->MoveIgnoreActorAdd(this);
    }
}

void AUR_Projectile::FireAt(const FVector& ShootDirection)
{
    ProjectileMovementComponent->Velocity = ShootDirection * ProjectileMovementComponent->InitialSpeed;
}

void AUR_Projectile::Overlap(UPrimitiveComponent * HitComp, AActor * Other, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
    // TODO
}

void AUR_Projectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
    // Old RocketProjectile code
    /*
    SoundHit->SetActive(true);
    SoundFire = UGameplayStatics::SpawnSoundAtLocation(this, SoundHit->Sound, this->GetActorLocation(), FRotator::ZeroRotator, 1.0f, 1.0f, 0.0f, nullptr, nullptr, true);
    Particles->SetTemplate(explosion);

    OtherActor->TakeDamage(100, FDamageEvent::FDamageEvent() , NULL, this);
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Damage Event ROCKET LAUNCHER")));
    if (ExplosionComponent->IsOverlappingActor(OtherActor))
        DamageNearActors();

    ProjMesh->DestroyComponent();
    DestroyAfter(3);
    */

    // Some notes :
    // - SoundFire => ImpactSound
    // - ProjMesh => StaticMeshComponent
    // - Particles => now spawning independent emitter with ImpactTemplate.

    if (GetNetMode() != NM_DedicatedServer)
    {
        //TODO: attenuation & concurrency settings
        UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());

        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            ImpactTemplate,
            FTransform(Hit.ImpactNormal.Rotation(), Hit.Location, GetActorScale3D())
        );
    }

    if (SplashRadius > 0.0f)
    {
        TArray<AActor*> IgnoreActors;
        IgnoreActors.Add(this);

        UGameplayStatics::ApplyRadialDamageWithFalloff(
            this,
            BaseDamage,
            SplashMinimumDamage,
            GetActorLocation(),
            InnerSplashRadius,
            SplashRadius,
            SplashFalloff,
            DamageTypeClass,
            IgnoreActors,
            this,
            GetInstigatorController(),
            ECollisionChannel::ECC_Visibility
        );
    }
    else
    {
        //TODO: not sure what to put in HitFromDirection.
        FVector HitFromDirection = (GetActorLocation() - Hit.Location);
        if (!HitFromDirection.IsNearlyZero())
        {
            HitFromDirection.Normalize();
        }
        else
        {
            HitFromDirection = -1 * GetActorRotation().Vector();
        }
        UGameplayStatics::ApplyPointDamage(OtherActor, BaseDamage, HitFromDirection, Hit, GetInstigatorController(), this, DamageTypeClass);
    }

    Destroy();
}

// might be deprecated
void AUR_Projectile::DestroyAfter(const int32 Delay)
{
    SetActorEnableCollision(false);
    SetLifeSpan(Delay);
}
