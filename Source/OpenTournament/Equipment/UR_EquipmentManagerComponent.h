// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UR_AbilitySet.h"
#include "Components/PawnComponent.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "UR_EquipmentManagerComponent.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UActorComponent;
class UUR_AbilitySystemComponent;
class UUR_EquipmentDefinition;
class UUR_EquipmentInstance;
class UUR_EquipmentManagerComponent;
class UObject;
struct FFrame;
struct FUR_EquipmentList;
struct FNetDeltaSerializeInfo;
struct FReplicationFlags;

/////////////////////////////////////////////////////////////////////////////////////////////////

/** A single piece of applied equipment */
USTRUCT(BlueprintType)
struct FUR_AppliedEquipmentEntry : public FFastArraySerializerItem
{
    GENERATED_BODY()

    FUR_AppliedEquipmentEntry()
    {}

    FString GetDebugString() const;

private:
    friend FUR_EquipmentList;
    friend UUR_EquipmentManagerComponent;

    // The equipment class that got equipped
    UPROPERTY()
    TSubclassOf<UUR_EquipmentDefinition> EquipmentDefinition;

    UPROPERTY()
    TObjectPtr<UUR_EquipmentInstance> Instance = nullptr;

    // Authority-only list of granted handles
    UPROPERTY(NotReplicated)
    FUR_AbilitySet_GrantedHandles GrantedHandles;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

/** List of applied equipment */
USTRUCT(BlueprintType)
struct FUR_EquipmentList : public FFastArraySerializer
{
    GENERATED_BODY()

    FUR_EquipmentList()
        : OwnerComponent(nullptr)
    {}

    FUR_EquipmentList(UActorComponent* InOwnerComponent)
        : OwnerComponent(InOwnerComponent)
    {}

public:
    //~FFastArraySerializer contract
    void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
    void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
    void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
    //~End of FFastArraySerializer contract

    bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
    {
        return FFastArraySerializer::FastArrayDeltaSerialize<FUR_AppliedEquipmentEntry, FUR_EquipmentList>(Entries, DeltaParms, *this);
    }

    UUR_EquipmentInstance* AddEntry(TSubclassOf<UUR_EquipmentDefinition> EquipmentDefinition);

    void RemoveEntry(UUR_EquipmentInstance* Instance);

private:
    UUR_AbilitySystemComponent* GetAbilitySystemComponent() const;

    friend UUR_EquipmentManagerComponent;

private:
    // Replicated list of equipment entries
    UPROPERTY()
    TArray<FUR_AppliedEquipmentEntry> Entries;

    UPROPERTY(NotReplicated)
    TObjectPtr<UActorComponent> OwnerComponent;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

template <>
struct TStructOpsTypeTraits<FUR_EquipmentList> : public TStructOpsTypeTraitsBase2<FUR_EquipmentList>
{
    enum { WithNetDeltaSerializer = true };
};

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Manages equipment applied to a pawn
 */
UCLASS(BlueprintType, Const)
class UUR_EquipmentManagerComponent : public UPawnComponent
{
    GENERATED_BODY()

public:
    UUR_EquipmentManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
    UUR_EquipmentInstance* EquipItem(TSubclassOf<UUR_EquipmentDefinition> EquipmentDefinition);

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
    void UnequipItem(UUR_EquipmentInstance* ItemInstance);

    //~UObject interface
    virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
    //~End of UObject interface

    //~UActorComponent interface
    virtual void InitializeComponent() override;
    virtual void UninitializeComponent() override;
    virtual void ReadyForReplication() override;
    //~End of UActorComponent interface

    /** Returns the first equipped instance of a given type, or nullptr if none are found */
    UFUNCTION(BlueprintCallable, BlueprintPure)
    UUR_EquipmentInstance* GetFirstInstanceOfType(TSubclassOf<UUR_EquipmentInstance> InstanceType);

    /** Returns all equipped instances of a given type, or an empty array if none are found */
    UFUNCTION(BlueprintCallable, BlueprintPure)
    TArray<UUR_EquipmentInstance*> GetEquipmentInstancesOfType(TSubclassOf<UUR_EquipmentInstance> InstanceType) const;

    template <typename T>
    T* GetFirstInstanceOfType()
    {
        return (T*)GetFirstInstanceOfType(T::StaticClass());
    }

private:
    UPROPERTY(Replicated)
    FUR_EquipmentList EquipmentList;
};
