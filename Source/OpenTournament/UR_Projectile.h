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
	// Sets default values for this actor's properties
	AUR_Projectile(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** weapon mesh: 3rd person view */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	UStaticMeshComponent* ProjMesh;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	UStaticMeshComponent* SM_TBox;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	class UParticleSystemComponent* Particles;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	UAudioComponent* SoundFire;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	UAudioComponent* SoundHit;


public:	

	virtual void Tick(float DeltaTime) override;

	// Sphere collision component.
	UPROPERTY(VisibleDefaultsOnly, Category = "Projectile")
	USphereComponent* CollisionComponent;

	// Projectile movement component.
	UPROPERTY(VisibleAnywhere, Category = Movement)
	UProjectileMovementComponent* ProjectileMovementComponent;

	void FireAt(const FVector& ShootDirection);

	void DestroyAfter(int delay);

};
