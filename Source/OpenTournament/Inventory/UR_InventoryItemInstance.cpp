// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_InventoryItemInstance.h"

#include "Inventory/UR_InventoryItemDefinition.h"
#include "Net/UnrealNetwork.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_InventoryItemInstance)

/////////////////////////////////////////////////////////////////////////////////////////////////

class FLifetimeProperty;

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_InventoryItemInstance::UUR_InventoryItemInstance(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UUR_InventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, StatTags);
    DOREPLIFETIME(ThisClass, ItemDef);
}

#if UE_WITH_IRIS
void UUR_InventoryItemInstance::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
    using namespace UE::Net;

    // Build descriptors and allocate PropertyReplicationFragments for this object
    FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif // UE_WITH_IRIS

void UUR_InventoryItemInstance::AddStatTagStack(FGameplayTag Tag, int32 StackCount)
{
    StatTags.AddStack(Tag, StackCount);
}

void UUR_InventoryItemInstance::RemoveStatTagStack(FGameplayTag Tag, int32 StackCount)
{
    StatTags.RemoveStack(Tag, StackCount);
}

int32 UUR_InventoryItemInstance::GetStatTagStackCount(FGameplayTag Tag) const
{
    return StatTags.GetStackCount(Tag);
}

bool UUR_InventoryItemInstance::HasStatTag(FGameplayTag Tag) const
{
    return StatTags.ContainsTag(Tag);
}

void UUR_InventoryItemInstance::SetItemDef(TSubclassOf<UUR_InventoryItemDefinition> InDef)
{
    ItemDef = InDef;
}

const UUR_InventoryItemFragment* UUR_InventoryItemInstance::FindFragmentByClass(TSubclassOf<UUR_InventoryItemFragment> FragmentClass) const
{
    if ((ItemDef != nullptr) && (FragmentClass != nullptr))
    {
        return GetDefault<UUR_InventoryItemDefinition>(ItemDef)->FindFragmentByClass(FragmentClass);
    }

    return nullptr;
}
