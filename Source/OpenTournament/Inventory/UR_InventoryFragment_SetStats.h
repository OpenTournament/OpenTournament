// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameplayTagContainer.h"
#include "Inventory/UR_InventoryItemDefinition.h"

#include "UR_InventoryFragment_SetStats.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UUR_InventoryItemInstance;
class UObject;
struct FGameplayTag;

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class UR_InventoryFragment_SetStats : public UUR_InventoryItemFragment
{
    GENERATED_BODY()

protected:
    UPROPERTY(EditDefaultsOnly, Category=Equipment)
    TMap<FGameplayTag, int32> InitialItemStats;

public:
    virtual void OnInstanceCreated(UUR_InventoryItemInstance* Instance) const override;

    int32 GetItemStatByTag(FGameplayTag Tag) const;
};
