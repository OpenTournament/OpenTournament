// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_InventoryItemDefinition.h"

#include "Templates/SubclassOf.h"
#include "UObject/ObjectPtr.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_InventoryItemDefinition)

/////////////////////////////////////////////////////////////////////////////////////////////////
// UUR_InventoryItemDefinition

UUR_InventoryItemDefinition::UUR_InventoryItemDefinition(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

const UUR_InventoryItemFragment* UUR_InventoryItemDefinition::FindFragmentByClass(TSubclassOf<UUR_InventoryItemFragment> FragmentClass) const
{
    if (FragmentClass != nullptr)
    {
        for (UUR_InventoryItemFragment* Fragment : Fragments)
        {
            if (Fragment && Fragment->IsA(FragmentClass))
            {
                return Fragment;
            }
        }
    }

    return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// UUR_InventoryItemDefinition

const UUR_InventoryItemFragment* UUR_InventoryFunctionLibrary::FindItemDefinitionFragment(TSubclassOf<UUR_InventoryItemDefinition> ItemDef, TSubclassOf<UUR_InventoryItemFragment> FragmentClass)
{
    if ((ItemDef != nullptr) && (FragmentClass != nullptr))
    {
        return GetDefault<UUR_InventoryItemDefinition>(ItemDef)->FindFragmentByClass(FragmentClass);
    }
    return nullptr;
}
