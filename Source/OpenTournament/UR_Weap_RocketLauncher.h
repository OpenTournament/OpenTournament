// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ConstructorHelpers.h"
#include "UR_Weapon.h"
#include "UR_Projectile_Rocket.h"

#include "UR_Weap_RocketLauncher.generated.h"

/**
 * 
 */

class AUR_Weap_RocketLauncher;


UCLASS()
class OPENTOURNAMENT_API AUR_Weap_RocketLauncher : public AUR_Weapon
{
	GENERATED_BODY()

	// Sets default values for this actor's properties
	AUR_Weap_RocketLauncher(const FObjectInitializer& ObjectInitializer);

	TSubclassOf<class AUR_Projectile_Rocket> ProjectileClass;
	class UParticleSystemComponent* Particles;

	virtual void Fire(UWorld* World, FVector MuzzleLocation, FRotator MuzzleRotation, FActorSpawnParameters SpawnParams) override;
	void BeginPlay();
};


