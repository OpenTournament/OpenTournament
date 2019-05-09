// Copyright 2019 Open Tournament Project, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "UR_Weapon.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////


/**
 *
 */
UCLASS()
class OPENTOURNAMENT_API AUR_Weapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AUR_Weapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
