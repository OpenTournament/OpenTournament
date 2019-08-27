// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ConstructorHelpers.h"
#include "UR_Projectile.h"
#include "UR_Character.h"
#include "UR_PlayerController.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystemComponent.h"
#include "UR_Projectile_Rocket.generated.h"

/**
*
*/

class AUR_Projectile_Rocket;

UCLASS()
class OPENTOURNAMENT_API AUR_Projectile_Rocket : public AUR_Projectile
{
	GENERATED_BODY()

	// Sets default values for this actor's properties
	AUR_Projectile_Rocket(const FObjectInitializer& ObjectInitializer);

	class UParticleSystemComponent* Particles;

	UParticleSystem* trail;
	UParticleSystem* explosion;

	// Sphere collision component.
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	USphereComponent* ExplosionComponent;

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	void DamageNearActors();


	void BeginPlay();
};
