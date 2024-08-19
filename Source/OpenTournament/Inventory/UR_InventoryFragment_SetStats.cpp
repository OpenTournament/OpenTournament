// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_InventoryFragment_SetStats.h"

#include "Inventory/UR_InventoryItemInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_InventoryFragment_SetStats)

/////////////////////////////////////////////////////////////////////////////////////////////////

void UR_InventoryFragment_SetStats::OnInstanceCreated(UUR_InventoryItemInstance* Instance) const
{
    for (const auto& KVP : InitialItemStats)
    {
        Instance->AddStatTagStack(KVP.Key, KVP.Value);
    }
}

int32 UR_InventoryFragment_SetStats::GetItemStatByTag(FGameplayTag Tag) const
{
    if (const int32* StatPtr = InitialItemStats.Find(Tag))
    {
        return *StatPtr;
    }

    return 0;
}
