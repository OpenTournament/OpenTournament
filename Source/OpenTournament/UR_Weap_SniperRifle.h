// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ConstructorHelpers.h"
#include "UR_Weapon.h"
#include "UR_Weap_SniperRifle.generated.h"

/**
*
*/

class AUR_Weap_SniperRifle;

UCLASS()
class OPENTOURNAMENT_API AUR_Weap_SniperRifle : public AUR_Weapon
{
	GENERATED_BODY()

		// Sets default values for this actor's properties
		AUR_Weap_SniperRifle(const FObjectInitializer& ObjectInitializer);

};
