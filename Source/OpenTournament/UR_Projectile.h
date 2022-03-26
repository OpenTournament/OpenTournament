// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameFramework/Actor.h"

#include "UR_Projectile.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations

class UAudioComponent;
class USphereComponent;
class UStaticMeshComponent;
class UParticleSystemComponent;
class UPrimitiveComponent;
class UProjectileMovementComponent;

/////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct FReplicatedExplosionInfo
{
    GENERATED_BODY()

    UPROPERTY()
    FVector HitLocation;

    UPROPERTY()
    FVector HitNormal;

    FReplicatedExplosionInfo() : HitLocation(0, 0, 0), HitNormal(0, 0, 0) {}
};

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class OPENTOURNAMENT_API AUR_Projectile : public AActor
{
    GENERATED_BODY()

public:
    AUR_Projectile(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;

    /////////////////////////////////////////////////////////////////////////////////////////////////

public:

    // Sphere collision component.
    UPROPERTY(VisibleDefaultsOnly, Category = "Projectile|Collision")
    USphereComponent* CollisionComponent;

    // Projectile movement component.
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Projectile|Movement")
    UProjectileMovementComponent* ProjectileMovementComponent;

    // Projectile Mesh
    UPROPERTY(VisibleDefaultsOnly, Category = "Projectile|Mesh")
    UStaticMeshComponent* StaticMeshComponent;

    // Audio Component
    UPROPERTY(VisibleDefaultsOnly, Category = "Projectile|Audio")
    UAudioComponent* AudioComponent;

    // Projectile Particles
    UPROPERTY(VisibleDefaultsOnly, Category = "Projectile|Particles")
    UParticleSystemComponent* Particles;

    /**
    * This should be enabled for lesser projectiles like spammable ones, to reduce network cost.
    * More prominent projectiles (grenades, rockets) should be false imo for more accurate simulations.
    * Interactive projectiles (combo balls) MUST be false to function properly.
    */
    UPROPERTY(EditAnywhere, Category = "Replication")
    bool bCutReplicationAfterSpawn;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Set projectile to not collide with shooter.
    * This should always be true by default, otherwise projectile can collide shooter on spawn.
    * Can be updated on the fly.
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Collision")
    bool bIgnoreInstigator;

    /**
    * Only for bouncing projectiles.
    * Use this to let projectile collide with shooter after first bounce.
    */
    UPROPERTY(EditAnywhere, Category = "Projectile|Collision")
    bool bCollideInstigatorAfterBounce;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    UFUNCTION()
    virtual void FireAt(const FVector& ShootDirection);

    UFUNCTION(BlueprintNativeEvent, Category = "Projectile")
    void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    /**
    * On overlap, return whether we should explode or continue through.
    */
    UFUNCTION(BlueprintNativeEvent, Category = "Projectile")
    bool OverlapShouldExplodeOn(AActor* Other);

    UFUNCTION()
    virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

    UFUNCTION()
    virtual void OnBounceInternal(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

    /**
    * On hit, only for bouncing projectiles.
    * Return whether we should explode on the hit actor, or bounce off it.
    * (Non-bouncing projectiles always explode on hit)
    */
    UFUNCTION(BlueprintNativeEvent, Category = "Projectile")
    bool HitShouldExplodeOn(AActor* Other);

    /**
    * Deal BaseDamage as point damage to actor.
    */
    UFUNCTION(BlueprintCallable, Category = "Projectile")
    void DealPointDamage(AActor* HitActor, const FHitResult& HitInfo);

    /**
    * Deal splash damage.
    */
    UFUNCTION(BlueprintCallable, Category = "Projectile")
    void DealSplashDamage();

    /**
    * Authority: replicate explosion & delayed destroy.
    * Client: play impact effects & destroy.
    */
    UFUNCTION(BlueprintCallable, Category = "Projectile")
    virtual void Explode(const FVector& HitLocation, const FVector& HitNormal);

    /**
    * Client only.
    * Play explosion/impact effects.
    * Hit location & normal can be used to alter/orient the explosion.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintCosmetic, Category = "Projectile")
    void PlayImpactEffects(const FVector& HitLocation, const FVector& HitNormal);

    /////////////////////////////////////////////////////////////////////////////////////////////////

protected:

    /**
     * Replicate this var to ensure that when projectile explodes on server, it explodes on clients as well.
     * Most of the time however, it is expected to explode on client beforehand.
     *
     * Sometimes, it might explode on client without exploding on server (unreg).
     * Not sure how to fix unless we skip overlaps on client, but that will harm smoothness of game.
     *
     * NOTE: We use a var instead of a multicast explode,
     * because multicast apparently re-creates the projectile on clients where it already exploded...
     */
    UPROPERTY(ReplicatedUsing = OnRep_ServerExplosionInfo)
    FReplicatedExplosionInfo ServerExplosionInfo;

    UPROPERTY()
    float ClientExplosionTime;

    UFUNCTION()
    virtual void OnRep_ServerExplosionInfo();

    /////////////////////////////////////////////////////////////////////////////////////////////////

public:

    /**
    * Damage for direct hits and for actors within InnerSplashRadius if applicable.
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Damage")
    float BaseDamage;

    /**
    * Radius for Damage
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Damage")
    float SplashRadius;

    /**
    * Radius within which no falloff is applied yet (Damage = BaseDamage).
    * Only if SplashRadius > 0.
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Damage")
    float InnerSplashRadius;

    /**
    * Minimum damage when somebody is hit at the edge of splash radius.
    * Only if SplashRadius > 0.
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Damage")
    float SplashMinimumDamage;

    /**
    * Splash damage falloff exponent.
    * Only if SplashRadius > 0.
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Damage")
    float SplashFalloff;

    /**
    * DamageType
    */
    UPROPERTY(EditAnywhere, Category = "Projectile|Damage")
    TSubclassOf<UDamageType> DamageTypeClass;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Bounce sound.
    */
    UPROPERTY(EditAnywhere, Category = "Projectile|Audio")
    USoundBase* BounceSound;

    /**
    * Avoid sound spam when rolling on ground.
    */
    UPROPERTY(EditAnywhere, Category = "Projectile|Audio")
    float BounceSoundVelocityThreshold;

    /**
    * Impact/explosion sound.
    */
    UPROPERTY(EditAnywhere, Category = "Projectile|Audio")
    USoundBase* ImpactSound;

    /**
    * Impact/explosion effect template.
    */
    UPROPERTY(EditAnywhere, Category = "Projectile|Particles")
    UParticleSystem* ImpactTemplate;
};
