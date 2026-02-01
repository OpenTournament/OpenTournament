// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Inventory/UR_InventoryItemDefinition.h"

#include "UR_InventoryFragment_ReticleConfig.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UUR_ReticleWidgetBase;
class UObject;

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class UUR_InventoryFragment_ReticleConfig : public UUR_InventoryItemFragment
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Reticle)
    TArray<TSubclassOf<UUR_ReticleWidgetBase>> ReticleWidgets;
};
