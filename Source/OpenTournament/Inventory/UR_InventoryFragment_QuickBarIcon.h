// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Inventory/UR_InventoryItemDefinition.h"
#include "Styling/SlateBrush.h"

#include "UR_InventoryFragment_QuickBarIcon.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UObject;

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class UUR_InventoryFragment_QuickBarIcon : public UUR_InventoryItemFragment
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
    FSlateBrush Brush;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
    FSlateBrush AmmoBrush;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
    FText DisplayNameWhenEquipped;
};
