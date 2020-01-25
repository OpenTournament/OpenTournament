// Fill out your copyright notice in the Description page of Project Settings.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "UR_Projectile.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations

class UAudioComponent;
class USphereComponent;
class UStaticMeshComponent;
class UParticleSystemComponent;
class UProjectileMovementComponent;

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class OPENTOURNAMENT_API AUR_Projectile : public AActor
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;

public:

    AUR_Projectile(const FObjectInitializer& ObjectInitializer);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    // Sphere collision component.
    UPROPERTY(VisibleDefaultsOnly, Category = "Projectile|Collision")
    USphereComponent* CollisionComponent;

    /**
    * Set projectile to not collide with shooter.
    * This should always be true by default, otherwise projectile can collide shooter on spawn.
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Collision")
    bool bIgnoreInstigator;

    /**
    * Only for bouncing projectiles.
    * Use this to let projectile collide with shooter after first bounce.
    */
    UPROPERTY(EditAnywhere, Category = "Projectile|Collision")
    bool bCollideInstigatorAfterBounce;

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

    /////////////////////////////////////////////////////////////////////////////////////////////////

    UFUNCTION(BlueprintCallable)
    virtual void SetIgnoreInstigator(bool bIgnore);

    UFUNCTION()
    virtual void FireAt(const FVector& ShootDirection);

    UFUNCTION()
    void Overlap(class UPrimitiveComponent* HitComp, class AActor* Other, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    // Hook for Blueprint. This will need to be elaborated further
    UFUNCTION(BlueprintImplementableEvent, Category = "Projectile")
    void OnOverlap(AActor* HitActor);

    //NOTE: Currently we are using this, not the Overlap events.
    UFUNCTION()
    virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

    UFUNCTION()
    virtual void OnBounceInternal(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

    /**
    * Only for bouncing projectiles.
    * Return whether we should explode on the hit actor, or bounce off it.
    */
    UFUNCTION(BlueprintNativeEvent)
    bool ShouldExplodeOn(AActor* Other);

    /**
    * Call this to trigger explosion/impact effects and splash damage.
    * If there is no splash radius, no damage is applied. Point damage should be applied on hit.
    */
    UFUNCTION(BlueprintCallable, Category = "Projectile")
    virtual void Explode(const FVector& HitLocation, const FVector& HitNormal);

    UFUNCTION()
    virtual void DestroyAfter(const int32 Delay);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    

    /**
    * Damage for direct hits and for actors within InnerSplashRadius if applicable.
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Damage")
    float BaseDamage;

    /**
    * 
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
    *
    */
    UPROPERTY(EditAnywhere, Category = "Projectile|Damage")
    TSubclassOf<UDamageType> DamageTypeClass;

    /////////////////////////////////////////////////////////////////////////////////////////////////

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
