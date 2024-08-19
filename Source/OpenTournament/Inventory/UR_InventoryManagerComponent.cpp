// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_InventoryManagerComponent.h"

#include "NativeGameplayTags.h"
#include "Engine/ActorChannel.h"
#include "Engine/World.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Net/UnrealNetwork.h"

#include "UR_InventoryItemDefinition.h"
#include "UR_InventoryItemInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_InventoryManagerComponent)

/////////////////////////////////////////////////////////////////////////////////////////////////

class FLifetimeProperty;
struct FReplicationFlags;

/////////////////////////////////////////////////////////////////////////////////////////////////

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UR__Inventory_Message_StackChanged, "OT.Inventory.Message.StackChanged");

//////////////////////////////////////////////////////////////////////
// FUR_InventoryEntry

FString FUR_InventoryEntry::GetDebugString() const
{
    TSubclassOf<UUR_InventoryItemDefinition> ItemDef;
    if (Instance != nullptr)
    {
        ItemDef = Instance->GetItemDef();
    }

    return FString::Printf(TEXT("%s (%d x %s)"), *GetNameSafe(Instance), StackCount, *GetNameSafe(ItemDef));
}

//////////////////////////////////////////////////////////////////////
// FUR_InventoryList

void FUR_InventoryList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
    for (int32 Index : RemovedIndices)
    {
        FUR_InventoryEntry& Stack = Entries[Index];
        BroadcastChangeMessage(Stack, /*OldCount=*/ Stack.StackCount, /*NewCount=*/ 0);
        Stack.LastObservedCount = 0;
    }
}

void FUR_InventoryList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
    for (int32 Index : AddedIndices)
    {
        FUR_InventoryEntry& Stack = Entries[Index];
        BroadcastChangeMessage(Stack, /*OldCount=*/ 0, /*NewCount=*/ Stack.StackCount);
        Stack.LastObservedCount = Stack.StackCount;
    }
}

void FUR_InventoryList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
    for (int32 Index : ChangedIndices)
    {
        FUR_InventoryEntry& Stack = Entries[Index];
        check(Stack.LastObservedCount != INDEX_NONE);
        BroadcastChangeMessage(Stack, /*OldCount=*/ Stack.LastObservedCount, /*NewCount=*/ Stack.StackCount);
        Stack.LastObservedCount = Stack.StackCount;
    }
}

void FUR_InventoryList::BroadcastChangeMessage(FUR_InventoryEntry& Entry, int32 OldCount, int32 NewCount)
{
    FUR_InventoryChangeMessage Message;
    Message.InventoryOwner = OwnerComponent;
    Message.Instance = Entry.Instance;
    Message.NewCount = NewCount;
    Message.Delta = NewCount - OldCount;

    UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(OwnerComponent->GetWorld());
    MessageSystem.BroadcastMessage(TAG_UR__Inventory_Message_StackChanged, Message);
}

UUR_InventoryItemInstance* FUR_InventoryList::AddEntry(TSubclassOf<UUR_InventoryItemDefinition> ItemDef, int32 StackCount)
{
    UUR_InventoryItemInstance* Result = nullptr;

    check(ItemDef != nullptr);
    check(OwnerComponent);

    AActor* OwningActor = OwnerComponent->GetOwner();
    check(OwningActor->HasAuthority());


    FUR_InventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
    NewEntry.Instance = NewObject<UUR_InventoryItemInstance>(OwnerComponent->GetOwner());  //@TODO: Using the actor instead of component as the outer due to UE-127172
    NewEntry.Instance->SetItemDef(ItemDef);
    for (UUR_InventoryItemFragment* Fragment : GetDefault<UUR_InventoryItemDefinition>(ItemDef)->Fragments)
    {
        if (Fragment != nullptr)
        {
            Fragment->OnInstanceCreated(NewEntry.Instance);
        }
    }
    NewEntry.StackCount = StackCount;
    Result = NewEntry.Instance;

    //const UUR_InventoryItemDefinition* ItemCDO = GetDefault<UUR_InventoryItemDefinition>(ItemDef);
    MarkItemDirty(NewEntry);

    return Result;
}

void FUR_InventoryList::AddEntry(UUR_InventoryItemInstance* Instance)
{
    unimplemented();
}

void FUR_InventoryList::RemoveEntry(UUR_InventoryItemInstance* Instance)
{
    for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
    {
        FUR_InventoryEntry& Entry = *EntryIt;
        if (Entry.Instance == Instance)
        {
            EntryIt.RemoveCurrent();
            MarkArrayDirty();
        }
    }
}

TArray<UUR_InventoryItemInstance*> FUR_InventoryList::GetAllItems() const
{
    TArray<UUR_InventoryItemInstance*> Results;
    Results.Reserve(Entries.Num());
    for (const FUR_InventoryEntry& Entry : Entries)
    {
        if (Entry.Instance != nullptr) //@TODO: Would prefer to not deal with this here and hide it further?
        {
            Results.Add(Entry.Instance);
        }
    }
    return Results;
}

//////////////////////////////////////////////////////////////////////
// UUR_InventoryManagerComponent

UUR_InventoryManagerComponent::UUR_InventoryManagerComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
  , InventoryList(this)
{
    SetIsReplicatedByDefault(true);
}

void UUR_InventoryManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, InventoryList);
}

bool UUR_InventoryManagerComponent::CanAddItemDefinition(TSubclassOf<UUR_InventoryItemDefinition> ItemDef, int32 StackCount)
{
    //@TODO: Add support for stack limit / uniqueness checks / etc...
    return true;
}

UUR_InventoryItemInstance* UUR_InventoryManagerComponent::AddItemDefinition(TSubclassOf<UUR_InventoryItemDefinition> ItemDef, int32 StackCount)
{
    UUR_InventoryItemInstance* Result = nullptr;
    if (ItemDef != nullptr)
    {
        Result = InventoryList.AddEntry(ItemDef, StackCount);

        if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && Result)
        {
            AddReplicatedSubObject(Result);
        }
    }
    return Result;
}

void UUR_InventoryManagerComponent::AddItemInstance(UUR_InventoryItemInstance* ItemInstance)
{
    InventoryList.AddEntry(ItemInstance);
    if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && ItemInstance)
    {
        AddReplicatedSubObject(ItemInstance);
    }
}

void UUR_InventoryManagerComponent::RemoveItemInstance(UUR_InventoryItemInstance* ItemInstance)
{
    InventoryList.RemoveEntry(ItemInstance);

    if (ItemInstance && IsUsingRegisteredSubObjectList())
    {
        RemoveReplicatedSubObject(ItemInstance);
    }
}

TArray<UUR_InventoryItemInstance*> UUR_InventoryManagerComponent::GetAllItems() const
{
    return InventoryList.GetAllItems();
}

UUR_InventoryItemInstance* UUR_InventoryManagerComponent::FindFirstItemStackByDefinition(TSubclassOf<UUR_InventoryItemDefinition> ItemDef) const
{
    for (const FUR_InventoryEntry& Entry : InventoryList.Entries)
    {
        UUR_InventoryItemInstance* Instance = Entry.Instance;

        if (IsValid(Instance))
        {
            if (Instance->GetItemDef() == ItemDef)
            {
                return Instance;
            }
        }
    }

    return nullptr;
}

int32 UUR_InventoryManagerComponent::GetTotalItemCountByDefinition(TSubclassOf<UUR_InventoryItemDefinition> ItemDef) const
{
    int32 TotalCount = 0;
    for (const FUR_InventoryEntry& Entry : InventoryList.Entries)
    {
        UUR_InventoryItemInstance* Instance = Entry.Instance;

        if (IsValid(Instance))
        {
            if (Instance->GetItemDef() == ItemDef)
            {
                ++TotalCount;
            }
        }
    }

    return TotalCount;
}

bool UUR_InventoryManagerComponent::ConsumeItemsByDefinition(TSubclassOf<UUR_InventoryItemDefinition> ItemDef, int32 NumToConsume)
{
    AActor* OwningActor = GetOwner();
    if (!OwningActor || !OwningActor->HasAuthority())
    {
        return false;
    }

    //@TODO: N squared right now as there's no acceleration structure
    int32 TotalConsumed = 0;
    while (TotalConsumed < NumToConsume)
    {
        if (UUR_InventoryItemInstance* Instance = UUR_InventoryManagerComponent::FindFirstItemStackByDefinition(ItemDef))
        {
            InventoryList.RemoveEntry(Instance);
            ++TotalConsumed;
        }
        else
        {
            return false;
        }
    }

    return TotalConsumed == NumToConsume;
}

void UUR_InventoryManagerComponent::ReadyForReplication()
{
    Super::ReadyForReplication();

    // Register existing UUR_InventoryItemInstance
    if (IsUsingRegisteredSubObjectList())
    {
        for (const FUR_InventoryEntry& Entry : InventoryList.Entries)
        {
            UUR_InventoryItemInstance* Instance = Entry.Instance;

            if (IsValid(Instance))
            {
                AddReplicatedSubObject(Instance);
            }
        }
    }
}

bool UUR_InventoryManagerComponent::ReplicateSubobjects(UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
    bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

    for (FUR_InventoryEntry& Entry : InventoryList.Entries)
    {
        UUR_InventoryItemInstance* Instance = Entry.Instance;

        if (Instance && IsValid(Instance))
        {
            WroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
        }
    }

    return WroteSomething;
}

//////////////////////////////////////////////////////////////////////
//

// UCLASS(Abstract)
// class UUR_InventoryFilter : public UObject
// {
// public:
// 	virtual bool PassesFilter(UUR_InventoryItemInstance* Instance) const { return true; }
// };

// UCLASS()
// class UUR_InventoryFilter_HasTag : public UUR_InventoryFilter
// {
// public:
// 	virtual bool PassesFilter(UUR_InventoryItemInstance* Instance) const { return true; }
// };
