// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Projectile.h"

#include <Engine/World.h>
#include <GameFramework/Pawn.h>

#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_Projectile)

/////////////////////////////////////////////////////////////////////////////////////////////////

//NOTE: Maybe a BouncingProjectile subclass would be appropriate.

AUR_Projectile::AUR_Projectile(const FObjectInitializer& ObjectInitializer)
{
    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
    CollisionComponent->BodyInstance.SetCollisionProfileName(TEXT("Projectile"));
    CollisionComponent->SetGenerateOverlapEvents(true);
    CollisionComponent->InitSphereRadius(15.0f);
    CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnOverlap);
    CollisionComponent->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);
    bIgnoreInstigator = true;
    bCollideInstigatorAfterBounce = true;

    RootComponent = CollisionComponent;

    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
    ProjectileMovementComponent->SetUpdatedComponent(CollisionComponent);
    ProjectileMovementComponent->InitialSpeed = 5000.0f;
    ProjectileMovementComponent->MaxSpeed = 5000.0f;
    ProjectileMovementComponent->bRotationFollowsVelocity = true;
    ProjectileMovementComponent->ProjectileGravityScale = 0.f;
    ProjectileMovementComponent->bShouldBounce = false;

    StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
    StaticMeshComponent->SetupAttachment(RootComponent);
    StaticMeshComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

    AudioComponent = ObjectInitializer.CreateDefaultSubobject<UAudioComponent>(this, TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);

    Particles = ObjectInitializer.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("Particles"));
    Particles->SetupAttachment(RootComponent);

    SetCanBeDamaged(false);

    bReplicates = true;
    bCutReplicationAfterSpawn = false;
    ClientExplosionTime = -10.f;

    BaseDamage = 100.f;
    SplashRadius = 0.0f;
    InnerSplashRadius = 10.f;
    SplashMinimumDamage = 1.0f;
    SplashFalloff = 1.0f;
    DamageTypeClass = UDamageType::StaticClass();

    BounceSoundVelocityThreshold = 80.f;

    InitialLifeSpan = 15.f;
}

void AUR_Projectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(ThisClass, ServerExplosionInfo, COND_None);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Projectile::BeginPlay()
{
    bNetTemporary = bCutReplicationAfterSpawn;

    Super::BeginPlay();

    if (ProjectileMovementComponent->bShouldBounce)
    {
        ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &ThisClass::OnBounceInternal);
    }
}

//deprecated
void AUR_Projectile::FireAt(const FVector& ShootDirection)
{
    //ProjectileMovementComponent->Velocity = ProjectileMovementComponent->Velocity ShootDirection * ProjectileMovementComponent->InitialSpeed;
    //NOTE: This is useless, projectile already does that, given we spawn it with the right rotation.
}

void AUR_Projectile::OnOverlap_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OverlapShouldExplodeOn(OtherActor))
    {
        if (SplashRadius > 0.0f)
        {
            DealSplashDamage();
        }
        else
        {
            DealPointDamage(OtherActor, SweepResult);
        }

        Explode(bFromSweep ? FVector(SweepResult.Location.X, SweepResult.Location.Y, SweepResult.Location.Z) : GetActorLocation(), SweepResult.ImpactNormal);
    }
}

bool AUR_Projectile::OverlapShouldExplodeOn_Implementation(AActor* Other)
{
    //NOTE: here we can implement team projectiles going through teammates
    return Other && Other->CanBeDamaged() && (!bIgnoreInstigator || Other != Cast<APawn>(GetInstigator()));
}

void AUR_Projectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
    if (ProjectileMovementComponent->bShouldBounce && !HitShouldExplodeOn(OtherActor))
    {
        return;
    }

    if (SplashRadius > 0.0f)
    {
        DealSplashDamage();
    }
    else
    {
        DealPointDamage(OtherActor, Hit);
    }

    Explode(Hit.Location, Hit.ImpactNormal);
}

void AUR_Projectile::OnBounceInternal(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
    float BounceNormalVelocity = FMath::Abs(FVector::DotProduct(ImpactVelocity, ImpactResult.Normal));
    if (GetNetMode() != NM_DedicatedServer && BounceNormalVelocity > BounceSoundVelocityThreshold)
    {
        //TODO: attenuation & concurrency settings, unless we do that in BP/SoundCue?
        //TODO: might want to factor BounceNormalVelocity into the sound somehow
        UGameplayStatics::PlaySoundAtLocation(this, BounceSound, ImpactResult.Location);
    }

    if (bCollideInstigatorAfterBounce && bIgnoreInstigator)
    {
        bIgnoreInstigator = false;

        if (CollisionComponent->IsOverlappingActor(GetInstigator()))
        {
            //NOTE: at this point the projectile hasn't been rotated yet by the bounce.
            // We need to rotate manually so ApplyPointDamage gets the proper hit direction.
            FVector BouncedDir = GetActorRotation().Vector().MirrorByVector(ImpactResult.ImpactNormal);

            // Only do that if we are sure we are gonna pointdamage instigator.
            if (OverlapShouldExplodeOn(GetInstigator()) && SplashRadius <= 0.0f)
            {
                SetActorRotation(BouncedDir.Rotation());
            }

            // Now trigger a fake overlap
            FHitResult FakeHitInfo(GetInstigator(), nullptr, GetActorLocation(), -BouncedDir);
            OnOverlap(nullptr, GetInstigator(), nullptr, 0, true, FakeHitInfo);
        }

        /*
        TArray<FOverlapInfo> Overlaps;
        CollisionComponent->GetOverlapsWithActor(GetInstigator(), Overlaps);
        if (Overlaps.Num() > 0)
        {
            FOverlapInfo& Overlap = Overlaps[0];
            OnOverlap(nullptr, Overlap.OverlapInfo.GetActor(), Overlap.OverlapInfo.GetComponent(), Overlap.GetBodyIndex(), true, Overlap.OverlapInfo);
        }
        */
    }
}

bool AUR_Projectile::HitShouldExplodeOn_Implementation(AActor* Other)
{
    return Other && Other->CanBeDamaged();
}

void AUR_Projectile::DealPointDamage(AActor* HitActor, const FHitResult& HitInfo)
{
    UGameplayStatics::ApplyPointDamage(HitActor, BaseDamage, GetActorRotation().Vector(), HitInfo, GetInstigatorController(), this, DamageTypeClass);
}

void AUR_Projectile::DealSplashDamage()
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

/**
* When we are bNetTemporary, replication link is cut after spawn, and client is fully responsible for exploding/destroying projectile.
*
* When we are not, there are 4 explosion scenarios to handle :
*
* (1) Explode on client before receiving ServerExplosionInfo.
*     > Play explosion, hide projectile, and wait for server confirmation.
*     > We will either receive ServerExplosionInfo soon (confirm case), or much later (unreg case).
*
* (2) Explode on client after receiving ServerExplosionInfo.
*     > This should not happen because we disable simulation in case (3).
*
* (3) Receive ServerExplosionInfo before exploded on client.
*     > Play explosion, stop simulating and destroy when possible.
*     (note: client cannot destroy as long as replication link exists)
*
* (4) Receive ServerExplosionInfo after exploded on client.
*     > See case (1), this is either a confirmation, or an unreg case.
*     > In unreg case, we should play the real explosion now.
*/

void AUR_Projectile::OnRep_ServerExplosionInfo()
{
    //UKismetSystemLibrary::PrintString(this, TEXT("OnRep_ServerExplosionInfo"));

    // If we received server explosion later than 200ms after exploding on client
    if (auto World = GetWorld())
    {
        if (World->TimeSince(ClientExplosionTime) > 0.200f)
        {
            // Play explosion again
            Explode(ServerExplosionInfo.HitLocation, ServerExplosionInfo.HitNormal);
        }
    }

    // Either way, stop simulating and wait for server destruction (client cannot destroy networked actor).
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
    ProjectileMovementComponent->StopSimulating(FHitResult());
}

void AUR_Projectile::Explode(const FVector& HitLocation, const FVector& HitNormal)
{
    //UKismetSystemLibrary::PrintString(this, TEXT("Explode"));

    PlayImpactEffects(HitLocation, HitNormal);

    if (bNetTemporary || GetNetMode() == NM_Standalone)
    {
        Destroy();
        return;
    }

    if (HasAuthority())
    {
        //UKismetSystemLibrary::PrintString(this, TEXT("HasAuthority > ServerExplosionInfo"));

        ServerExplosionInfo.HitLocation = HitLocation;
        ServerExplosionInfo.HitNormal = HitNormal;
        ForceNetUpdate();

        SetActorEnableCollision(false);
        ProjectileMovementComponent->StopSimulating(FHitResult());

        // Give 200ms to replicate explosion before destroy
        SetLifeSpan(0.200f);
    }
    else
    {
        //UKismetSystemLibrary::PrintString(this, TEXT("Client > Hide"));

        ClientExplosionTime = GetWorld()->GetTimeSeconds();

        // On networked client, hide projectile and collision but continue simulating.
        // If we don't receive server confirmation, we might want to re-show projectile after a delay (TODO).
        SetActorHiddenInGame(true);
        SetActorEnableCollision(false);
    }
}

void AUR_Projectile::PlayImpactEffects_Implementation(const FVector& HitLocation, const FVector& HitNormal)
{
    if (GetNetMode() != NM_DedicatedServer)
    {
        //TODO: attenuation & concurrency settings, unless we do that in BP/SoundCue?
        UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, HitLocation);

        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            ImpactTemplate,
            FTransform(HitNormal.Rotation(), HitLocation, GetActorScale3D())
        );
    }
}
