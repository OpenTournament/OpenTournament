// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Inventory/UR_InventoryItemDefinition.h"
#include "Templates/SubclassOf.h"

#include "UR_InventoryFragment_EquippableItem.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UUR_EquipmentDefinition;
class UObject;

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class UUR_InventoryFragment_EquippableItem : public UUR_InventoryItemFragment
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category=UR_)
    TSubclassOf<UUR_EquipmentDefinition> EquipmentDefinition;
};
