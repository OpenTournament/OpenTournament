// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_GameplayAbility_FromEquipment.h"
#include "UR_EquipmentInstance.h"
#include "Inventory/UR_InventoryItemInstance.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_GameplayAbility_FromEquipment)

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_GameplayAbility_FromEquipment::UUR_GameplayAbility_FromEquipment(const FObjectInitializer& ObjectInitializer)
{
}

UUR_EquipmentInstance* UUR_GameplayAbility_FromEquipment::GetAssociatedEquipment() const
{
    if (FGameplayAbilitySpec* Spec = UGameplayAbility::GetCurrentAbilitySpec())
    {
        return Cast<UUR_EquipmentInstance>(Spec->SourceObject.Get());
    }

    return nullptr;
}

UUR_InventoryItemInstance* UUR_GameplayAbility_FromEquipment::GetAssociatedItem() const
{
    if (UUR_EquipmentInstance* Equipment = GetAssociatedEquipment())
    {
        return Cast<UUR_InventoryItemInstance>(Equipment->GetInstigator());
    }
    return nullptr;
}


#if WITH_EDITOR
EDataValidationResult UUR_GameplayAbility_FromEquipment::IsDataValid(FDataValidationContext& Context) const
{
    EDataValidationResult Result = Super::IsDataValid(Context);

    if (InstancingPolicy == EGameplayAbilityInstancingPolicy::InstancedPerActor)
    {
        Context.AddError(NSLOCTEXT("OT", "EquipmentAbilityMustBeInstanced", "Equipment ability must be instanced"));
        Result = EDataValidationResult::Invalid;
    }

    return Result;
}

#endif
