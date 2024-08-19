// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "UR_InventoryManagerComponent.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UUR_InventoryItemDefinition;
class UUR_InventoryItemInstance;
class UUR_InventoryManagerComponent;
class UObject;
struct FFrame;
struct FUR_InventoryList;
struct FNetDeltaSerializeInfo;
struct FReplicationFlags;

/////////////////////////////////////////////////////////////////////////////////////////////////

/** A message when an item is added to the inventory */
USTRUCT(BlueprintType)
struct FUR_InventoryChangeMessage
{
    GENERATED_BODY()

    //@TODO: Tag based names+owning actors for inventories instead of directly exposing the component?
    UPROPERTY(BlueprintReadOnly, Category=Inventory)
    TObjectPtr<UActorComponent> InventoryOwner = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = Inventory)
    TObjectPtr<UUR_InventoryItemInstance> Instance = nullptr;

    UPROPERTY(BlueprintReadOnly, Category=Inventory)
    int32 NewCount = 0;

    UPROPERTY(BlueprintReadOnly, Category=Inventory)
    int32 Delta = 0;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

/** A single entry in an inventory */
USTRUCT(BlueprintType)
struct FUR_InventoryEntry : public FFastArraySerializerItem
{
    GENERATED_BODY()

    FUR_InventoryEntry()
    {
    }

    FString GetDebugString() const;

private:
    friend FUR_InventoryList;
    friend UUR_InventoryManagerComponent;

    UPROPERTY()
    TObjectPtr<UUR_InventoryItemInstance> Instance = nullptr;

    UPROPERTY()
    int32 StackCount = 0;

    UPROPERTY(NotReplicated)
    int32 LastObservedCount = INDEX_NONE;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

/** List of inventory items */
USTRUCT(BlueprintType)
struct FUR_InventoryList : public FFastArraySerializer
{
    GENERATED_BODY()

    FUR_InventoryList()
        : OwnerComponent(nullptr)
    {
    }

    FUR_InventoryList(UActorComponent* InOwnerComponent)
        : OwnerComponent(InOwnerComponent)
    {
    }

    TArray<UUR_InventoryItemInstance*> GetAllItems() const;

public:
    //~FFastArraySerializer contract
    void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);

    void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);

    void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

    //~End of FFastArraySerializer contract

    bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
    {
        return FFastArraySerializer::FastArrayDeltaSerialize<FUR_InventoryEntry, FUR_InventoryList>(Entries, DeltaParms, *this);
    }

    UUR_InventoryItemInstance* AddEntry(TSubclassOf<UUR_InventoryItemDefinition> ItemClass, int32 StackCount);

    void AddEntry(UUR_InventoryItemInstance* Instance);

    void RemoveEntry(UUR_InventoryItemInstance* Instance);

private:
    void BroadcastChangeMessage(FUR_InventoryEntry& Entry, int32 OldCount, int32 NewCount);

private:
    friend UUR_InventoryManagerComponent;

private:
    // Replicated list of items
    UPROPERTY()
    TArray<FUR_InventoryEntry> Entries;

    UPROPERTY(NotReplicated)
    TObjectPtr<UActorComponent> OwnerComponent;
};

template <>
struct TStructOpsTypeTraits<FUR_InventoryList> : public TStructOpsTypeTraitsBase2<FUR_InventoryList>
{
    enum { WithNetDeltaSerializer = true };
};

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Manages an inventory
 */
UCLASS(BlueprintType)
class OPENTOURNAMENT_API UUR_InventoryManagerComponent
    : public UActorComponent
{
    GENERATED_BODY()

public:
    UUR_InventoryManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
    bool CanAddItemDefinition(TSubclassOf<UUR_InventoryItemDefinition> ItemDef, int32 StackCount = 1);

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
    UUR_InventoryItemInstance* AddItemDefinition(TSubclassOf<UUR_InventoryItemDefinition> ItemDef, int32 StackCount = 1);

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
    void AddItemInstance(UUR_InventoryItemInstance* ItemInstance);

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
    void RemoveItemInstance(UUR_InventoryItemInstance* ItemInstance);

    UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure=false)
    TArray<UUR_InventoryItemInstance*> GetAllItems() const;

    UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure)
    UUR_InventoryItemInstance* FindFirstItemStackByDefinition(TSubclassOf<UUR_InventoryItemDefinition> ItemDef) const;

    int32 GetTotalItemCountByDefinition(TSubclassOf<UUR_InventoryItemDefinition> ItemDef) const;

    bool ConsumeItemsByDefinition(TSubclassOf<UUR_InventoryItemDefinition> ItemDef, int32 NumToConsume);

    //~UObject interface
    virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

    virtual void ReadyForReplication() override;

    //~End of UObject interface

private:
    UPROPERTY(Replicated)
    FUR_InventoryList InventoryList;
};
