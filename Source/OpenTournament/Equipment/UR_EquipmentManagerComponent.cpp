// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_EquipmentManagerComponent.h"

#include "AbilitySystemGlobals.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"

#include "UR_EquipmentDefinition.h"
#include "UR_EquipmentInstance.h"
#include "GAS/UR_AbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_EquipmentManagerComponent)

/////////////////////////////////////////////////////////////////////////////////////////////////

class FLifetimeProperty;
struct FReplicationFlags;

//////////////////////////////////////////////////////////////////////
// FUR_AppliedEquipmentEntry

FString FUR_AppliedEquipmentEntry::GetDebugString() const
{
    return FString::Printf(TEXT("%s of %s"), *GetNameSafe(Instance), *GetNameSafe(EquipmentDefinition.Get()));
}

//////////////////////////////////////////////////////////////////////
// FUR_EquipmentList

void FUR_EquipmentList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
    for (int32 Index : RemovedIndices)
    {
        const FUR_AppliedEquipmentEntry& Entry = Entries[Index];
        if (Entry.Instance != nullptr)
        {
            Entry.Instance->OnUnequipped();
        }
    }
}

void FUR_EquipmentList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
    for (int32 Index : AddedIndices)
    {
        const FUR_AppliedEquipmentEntry& Entry = Entries[Index];
        if (Entry.Instance != nullptr)
        {
            Entry.Instance->OnEquipped();
        }
    }
}

void FUR_EquipmentList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
    // 	for (int32 Index : ChangedIndices)
    // 	{
    // 		const FGameplayTagStack& Stack = Stacks[Index];
    // 		TagToCountMap[Stack.Tag] = Stack.StackCount;
    // 	}
}

UUR_AbilitySystemComponent* FUR_EquipmentList::GetAbilitySystemComponent() const
{
    check(OwnerComponent);
    AActor* OwningActor = OwnerComponent->GetOwner();
    return Cast<UUR_AbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor));
}

UUR_EquipmentInstance* FUR_EquipmentList::AddEntry(TSubclassOf<UUR_EquipmentDefinition> EquipmentDefinition)
{
    UUR_EquipmentInstance* Result = nullptr;

    check(EquipmentDefinition != nullptr);
    check(OwnerComponent);
    check(OwnerComponent->GetOwner()->HasAuthority());

    const UUR_EquipmentDefinition* EquipmentCDO = GetDefault<UUR_EquipmentDefinition>(EquipmentDefinition);

    TSubclassOf<UUR_EquipmentInstance> InstanceType = EquipmentCDO->InstanceType;
    if (InstanceType == nullptr)
    {
        InstanceType = UUR_EquipmentInstance::StaticClass();
    }

    FUR_AppliedEquipmentEntry& NewEntry = Entries.AddDefaulted_GetRef();
    NewEntry.EquipmentDefinition = EquipmentDefinition;
    NewEntry.Instance = NewObject<UUR_EquipmentInstance>(OwnerComponent->GetOwner(), InstanceType);  //@TODO: Using the actor instead of component as the outer due to UE-127172
    Result = NewEntry.Instance;

    if (UUR_AbilitySystemComponent* ASC = GetAbilitySystemComponent())
    {
        for (const TObjectPtr<const UUR_AbilitySet>& AbilitySet : EquipmentCDO->AbilitySetsToGrant)
        {
            AbilitySet->GiveToAbilitySystem(ASC, /*inout*/ &NewEntry.GrantedHandles, Result);
        }
    }
    else
    {
        //@TODO: Warning logging?
    }

    Result->SpawnEquipmentActors(EquipmentCDO->ActorsToSpawn);


    MarkItemDirty(NewEntry);

    return Result;
}

void FUR_EquipmentList::RemoveEntry(UUR_EquipmentInstance* Instance)
{
    for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
    {
        FUR_AppliedEquipmentEntry& Entry = *EntryIt;
        if (Entry.Instance == Instance)
        {
            if (UUR_AbilitySystemComponent* ASC = GetAbilitySystemComponent())
            {
                Entry.GrantedHandles.TakeFromAbilitySystem(ASC);
            }

            Instance->DestroyEquipmentActors();


            EntryIt.RemoveCurrent();
            MarkArrayDirty();
        }
    }
}

//////////////////////////////////////////////////////////////////////
// UUR_EquipmentManagerComponent

UUR_EquipmentManagerComponent::UUR_EquipmentManagerComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , EquipmentList(this)
{
    SetIsReplicatedByDefault(true);
    bWantsInitializeComponent = true;
}

void UUR_EquipmentManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, EquipmentList);
}

UUR_EquipmentInstance* UUR_EquipmentManagerComponent::EquipItem(TSubclassOf<UUR_EquipmentDefinition> EquipmentClass)
{
    UUR_EquipmentInstance* Result = nullptr;
    if (EquipmentClass != nullptr)
    {
        Result = EquipmentList.AddEntry(EquipmentClass);
        if (Result != nullptr)
        {
            Result->OnEquipped();

            if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
            {
                AddReplicatedSubObject(Result);
            }
        }
    }
    return Result;
}

void UUR_EquipmentManagerComponent::UnequipItem(UUR_EquipmentInstance* ItemInstance)
{
    if (ItemInstance != nullptr)
    {
        if (IsUsingRegisteredSubObjectList())
        {
            RemoveReplicatedSubObject(ItemInstance);
        }

        ItemInstance->OnUnequipped();
        EquipmentList.RemoveEntry(ItemInstance);
    }
}

bool UUR_EquipmentManagerComponent::ReplicateSubobjects(UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
    bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

    for (FUR_AppliedEquipmentEntry& Entry : EquipmentList.Entries)
    {
        UUR_EquipmentInstance* Instance = Entry.Instance;
        if (IsValid(Instance))
        {
            WroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
        }
    }

    return WroteSomething;
}

void UUR_EquipmentManagerComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UUR_EquipmentManagerComponent::UninitializeComponent()
{
    TArray<UUR_EquipmentInstance*> AllEquipmentInstances;

    // gathering all instances before removal to avoid side effects affecting the equipment list iterator
    for (const FUR_AppliedEquipmentEntry& Entry : EquipmentList.Entries)
    {
        AllEquipmentInstances.Add(Entry.Instance);
    }

    for (UUR_EquipmentInstance* EquipInstance : AllEquipmentInstances)
    {
        UnequipItem(EquipInstance);
    }

    Super::UninitializeComponent();
}

void UUR_EquipmentManagerComponent::ReadyForReplication()
{
    Super::ReadyForReplication();

    // Register existing UR_EquipmentInstances
    if (IsUsingRegisteredSubObjectList())
    {
        for (const FUR_AppliedEquipmentEntry& Entry : EquipmentList.Entries)
        {
            UUR_EquipmentInstance* Instance = Entry.Instance;
            if (IsValid(Instance))
            {
                AddReplicatedSubObject(Instance);
            }
        }
    }
}

UUR_EquipmentInstance* UUR_EquipmentManagerComponent::GetFirstInstanceOfType(TSubclassOf<UUR_EquipmentInstance> InstanceType)
{
    for (FUR_AppliedEquipmentEntry& Entry : EquipmentList.Entries)
    {
        if (UUR_EquipmentInstance* Instance = Entry.Instance)
        {
            if (Instance->IsA(InstanceType))
            {
                return Instance;
            }
        }
    }

    return nullptr;
}

TArray<UUR_EquipmentInstance*> UUR_EquipmentManagerComponent::GetEquipmentInstancesOfType(TSubclassOf<UUR_EquipmentInstance> InstanceType) const
{
    TArray<UUR_EquipmentInstance*> Results;
    for (const FUR_AppliedEquipmentEntry& Entry : EquipmentList.Entries)
    {
        if (UUR_EquipmentInstance* Instance = Entry.Instance)
        {
            if (Instance->IsA(InstanceType))
            {
                Results.Add(Instance);
            }
        }
    }
    return Results;
}
