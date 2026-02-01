// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_QuickBarComponent.h"

#include <NativeGameplayTags.h>
#include <GameFramework/GameplayMessageSubsystem.h>
#include <GameFramework/Pawn.h>
#include <Net/UnrealNetwork.h>

#include "Equipment/UR_EquipmentDefinition.h"
#include "Equipment/UR_EquipmentInstance.h"
#include "Equipment/UR_EquipmentManagerComponent.h"
#include "Inventory/UR_InventoryFragment_EquippableItem.h"
#include "Inventory/UR_InventoryItemInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_QuickBarComponent)

/////////////////////////////////////////////////////////////////////////////////////////////////

class FLifetimeProperty;
class UUR_EquipmentDefinition;

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_QuickBar_Message_SlotsChanged, "QuickBar.Message.SlotsChanged");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_QuickBar_Message_ActiveIndexChanged, "QuickBar.Message.ActiveIndexChanged");

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_QuickBarComponent::UUR_QuickBarComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , NumSlots(3)
{
    SetIsReplicatedByDefault(true);
}

void UUR_QuickBarComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, Slots);
    DOREPLIFETIME(ThisClass, ActiveSlotIndex);
}

void UUR_QuickBarComponent::BeginPlay()
{
    if (Slots.Num() < NumSlots)
    {
        Slots.AddDefaulted(NumSlots - Slots.Num());
    }

    Super::BeginPlay();
}

void UUR_QuickBarComponent::CycleActiveSlotForward()
{
    if (Slots.Num() < 2)
    {
        return;
    }

    const int32 OldIndex = (ActiveSlotIndex < 0 ? Slots.Num() - 1 : ActiveSlotIndex);
    int32 NewIndex = ActiveSlotIndex;
    do
    {
        NewIndex = (NewIndex + 1) % Slots.Num();
        if (Slots[NewIndex] != nullptr)
        {
            SetActiveSlotIndex(NewIndex);
            return;
        }
    }
    while (NewIndex != OldIndex);
}

void UUR_QuickBarComponent::CycleActiveSlotBackward()
{
    if (Slots.Num() < 2)
    {
        return;
    }

    const int32 OldIndex = (ActiveSlotIndex < 0 ? Slots.Num() - 1 : ActiveSlotIndex);
    int32 NewIndex = ActiveSlotIndex;
    do
    {
        NewIndex = (NewIndex - 1 + Slots.Num()) % Slots.Num();
        if (Slots[NewIndex] != nullptr)
        {
            SetActiveSlotIndex(NewIndex);
            return;
        }
    }
    while (NewIndex != OldIndex);
}

void UUR_QuickBarComponent::EquipItemInSlot()
{
    check(Slots.IsValidIndex(ActiveSlotIndex));
    check(EquippedItem == nullptr);

    if (UUR_InventoryItemInstance* SlotItem = Slots[ActiveSlotIndex])
    {
        if (const UUR_InventoryFragment_EquippableItem* EquipInfo = SlotItem->FindFragmentByClass<UUR_InventoryFragment_EquippableItem>())
        {
            TSubclassOf<UUR_EquipmentDefinition> EquipDef = EquipInfo->EquipmentDefinition;
            if (EquipDef != nullptr)
            {
                if (UUR_EquipmentManagerComponent* EquipmentManager = FindEquipmentManager())
                {
                    EquippedItem = EquipmentManager->EquipItem(EquipDef);
                    if (EquippedItem != nullptr)
                    {
                        EquippedItem->SetInstigator(SlotItem);
                    }
                }
            }
        }
    }
}

void UUR_QuickBarComponent::UnequipItemInSlot()
{
    if (UUR_EquipmentManagerComponent* EquipmentManager = FindEquipmentManager())
    {
        if (EquippedItem != nullptr)
        {
            EquipmentManager->UnequipItem(EquippedItem);
            EquippedItem = nullptr;
        }
    }
}

UUR_EquipmentManagerComponent* UUR_QuickBarComponent::FindEquipmentManager() const
{
    if (AController* OwnerController = Cast<AController>(GetOwner()))
    {
        if (APawn* Pawn = OwnerController->GetPawn())
        {
            return Pawn->FindComponentByClass<UUR_EquipmentManagerComponent>();
        }
    }
    return nullptr;
}

void UUR_QuickBarComponent::SetActiveSlotIndex_Implementation(int32 NewIndex)
{
    if (Slots.IsValidIndex(NewIndex) && (ActiveSlotIndex != NewIndex))
    {
        UnequipItemInSlot();

        ActiveSlotIndex = NewIndex;

        EquipItemInSlot();

        OnRep_ActiveSlotIndex();
    }
}

UUR_InventoryItemInstance* UUR_QuickBarComponent::GetActiveSlotItem() const
{
    return Slots.IsValidIndex(ActiveSlotIndex) ? Slots[ActiveSlotIndex] : nullptr;
}

int32 UUR_QuickBarComponent::GetNextFreeItemSlot() const
{
    int32 SlotIndex = 0;
    for (const TObjectPtr<UUR_InventoryItemInstance>& ItemPtr : Slots)
    {
        if (ItemPtr == nullptr)
        {
            return SlotIndex;
        }
        ++SlotIndex;
    }

    return INDEX_NONE;
}

void UUR_QuickBarComponent::AddItemToSlot(int32 SlotIndex, UUR_InventoryItemInstance* Item)
{
    if (Slots.IsValidIndex(SlotIndex) && (Item != nullptr))
    {
        if (Slots[SlotIndex] == nullptr)
        {
            Slots[SlotIndex] = Item;
            OnRep_Slots();
        }
    }
}

UUR_InventoryItemInstance* UUR_QuickBarComponent::RemoveItemFromSlot(int32 SlotIndex)
{
    UUR_InventoryItemInstance* Result = nullptr;

    if (ActiveSlotIndex == SlotIndex)
    {
        UnequipItemInSlot();
        ActiveSlotIndex = -1;
    }

    if (Slots.IsValidIndex(SlotIndex))
    {
        Result = Slots[SlotIndex];

        if (Result != nullptr)
        {
            Slots[SlotIndex] = nullptr;
            OnRep_Slots();
        }
    }

    return Result;
}

void UUR_QuickBarComponent::OnRep_Slots()
{
    FGameQuickBarSlotsChangedMessage Message;
    Message.Owner = GetOwner();
    Message.Slots = Slots;

    UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(this);
    MessageSystem.BroadcastMessage(TAG_QuickBar_Message_SlotsChanged, Message);
}

void UUR_QuickBarComponent::OnRep_ActiveSlotIndex()
{
    FGameQuickBarActiveIndexChangedMessage Message;
    Message.Owner = GetOwner();
    Message.ActiveIndex = ActiveSlotIndex;

    UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(this);
    MessageSystem.BroadcastMessage(TAG_QuickBar_Message_ActiveIndexChanged, Message);
}
