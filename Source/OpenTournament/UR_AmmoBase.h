// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "UR_PickupBase.h"

#include "UR_AmmoBase.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AUR_Ammo;
class USkeletalMesh;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Weapon Base
 */
UCLASS()
class OPENTOURNAMENT_API AUR_AmmoBase : public AUR_PickupBase
{
	GENERATED_BODY()

public:
	AUR_AmmoBase();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AUR_Ammo> AmmoClass;

protected:
#if WITH_EDITOR
	virtual void CheckForErrors() override;
#endif
	virtual void BeginPlay() override;

public:
	virtual void GiveTo_Implementation(class AActor* Other) override;
	virtual FText GetItemName_Implementation() override;

};
