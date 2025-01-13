// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GAS/UR_GameplayAbility.h"

#include "UR_GameplayAbility_FromEquipment.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UUR_EquipmentInstance;
class UUR_InventoryItemInstance;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * UUR_GameplayAbility_FromEquipment
 *
 * An ability granted by and associated with an equipment instance
 */
UCLASS()
class UUR_GameplayAbility_FromEquipment : public UUR_GameplayAbility
{
    GENERATED_BODY()

public:
    UUR_GameplayAbility_FromEquipment(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    UFUNCTION(BlueprintCallable, Category="OT|Ability")
    UUR_EquipmentInstance* GetAssociatedEquipment() const;

    UFUNCTION(BlueprintCallable, Category = "OT|Ability")
    UUR_InventoryItemInstance* GetAssociatedItem() const;

#if WITH_EDITOR
    virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
