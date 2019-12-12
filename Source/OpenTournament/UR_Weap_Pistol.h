// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ConstructorHelpers.h"
#include "UR_Weapon.h"
#include "UR_Projectile_Assault.h"

#include "UR_Weap_Pistol.generated.h"

/**
*
*/

class AUR_Weap_Pistol;

UCLASS()
class OPENTOURNAMENT_API AUR_Weap_Pistol : public AUR_Weapon
{
	GENERATED_BODY()

	// Sets default values for this actor's properties
	AUR_Weap_Pistol(const FObjectInitializer& ObjectInitializer);

	//TSubclassOf<class AUR_Projectile_Assault> ProjectileClass;

	virtual  void Fire(UWorld* World, FVector MuzzleLocation, FRotator MuzzleRotation, FActorSpawnParameters SpawnParams) override;

};
