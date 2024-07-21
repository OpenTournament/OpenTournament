// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "UR_PickupBase.h"
#include "UR_HealthBase.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UGameplayEffect;

/**
 * @! TODO: Deprecate Class due to bad naming & overlap with PickupFactory
 */
UCLASS(meta=(Deprecated))
class OPENTOURNAMENT_API AUR_HealthBase : public AUR_PickupBase
{
	GENERATED_BODY()

public:
	AUR_HealthBase();

	UPROPERTY(EditAnywhere)
	int32 HealAmount;

	UPROPERTY(EditAnywhere)
	bool bSuperHeal;

public:
	virtual bool AllowPickupBy_Implementation(class AActor* Other) override;
	virtual void GiveTo_Implementation(class AActor* Other) override;
	virtual FText GetItemName_Implementation() override;

    TSubclassOf<UGameplayEffect> EffectClass;
};
