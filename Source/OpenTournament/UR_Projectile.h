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

public:

    AUR_Projectile(const FObjectInitializer& ObjectInitializer);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    // Sphere collision component.
    UPROPERTY(VisibleDefaultsOnly, Category = "Projectile|Collision")
    USphereComponent* CollisionComponent;

    // Projectile movement component.
    UPROPERTY(VisibleAnywhere, Category = "Projectile|Movement")
    UProjectileMovementComponent* ProjectileMovementComponent;

    // Projectile Mesh
    UPROPERTY(VisibleDefaultsOnly, Category = "Projectile|Mesh")
    UStaticMeshComponent* StaticMeshComponent;

    // Audio Component
    UPROPERTY(VisibleDefaultsOnly, Category = "Projectile|Audio")
    UAudioComponent* AudioComponent;

    // Projectile Particles
    UPROPERTY(EditAnywhere, Category = "Projectile|Particles")
    UParticleSystemComponent* Particles;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    UFUNCTION()
    void Overlap(class UPrimitiveComponent* HitComp, class AActor* Other, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    // Hook for Blueprint. This will need to be elaborated further
    UFUNCTION(BlueprintImplementableEvent, Category = "Projectile")
    void OnOverlap(AActor* HitActor);

    void FireAt(const FVector& ShootDirection);

    void DestroyAfter(const int32 delay);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    UPROPERTY(EditAnywhere, Category = "Projectile|Audio")
    USoundBase* SoundHit;

    UPROPERTY()
    float Damage;

    UPROPERTY()
    float DamageRadius;
};
