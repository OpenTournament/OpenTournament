// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ConstructorHelpers.h"
#include "UR_Weapon.h"
#include "UR_Projectile_Assault.h"
#include "UR_Weap_AssaultRifle.generated.h"

/**
 *
 */

class AUR_Weap_AssaultRifle;

UCLASS()
class OPENTOURNAMENT_API AUR_Weap_AssaultRifle : public AUR_Weapon
{
	GENERATED_BODY()

	AUR_Weap_AssaultRifle(const FObjectInitializer& ObjectInitializer);

	//TSubclassOf<class AUR_Projectile_Assault> ProjectileClass;

	virtual void Fire(UWorld* World, FVector MuzzleLocation, FRotator MuzzleRotation, FActorSpawnParameters SpawnParams) override;
};
