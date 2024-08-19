// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Inventory/UR_InventoryItemDefinition.h"
#include "UObject/ObjectPtr.h"

#include "UR_InventoryFragment_PickupIcon.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UObject;
class USkeletalMesh;

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class UUR_InventoryFragment_PickupIcon : public UUR_InventoryItemFragment
{
    GENERATED_BODY()

public:
    UUR_InventoryFragment_PickupIcon();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
    TObjectPtr<USkeletalMesh> SkeletalMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
    FLinearColor PadColor;
};
