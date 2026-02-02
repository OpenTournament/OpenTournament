// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Components/ControllerComponent.h"

#include "UR_QuickBarComponent.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AActor;
class UUR_EquipmentInstance;
class UUR_EquipmentManagerComponent;
class UUR_InventoryItemInstance;
class UObject;
struct FFrame;

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class UUR_QuickBarComponent : public UControllerComponent
{
    GENERATED_BODY()

public:
    UUR_QuickBarComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    UFUNCTION(BlueprintCallable, Category="Game")
    void CycleActiveSlotForward();

    UFUNCTION(BlueprintCallable, Category="Game")
    void CycleActiveSlotBackward();

    UFUNCTION(Server, Reliable, BlueprintCallable, Category="Game")
    void SetActiveSlotIndex(int32 NewIndex);

    UFUNCTION(BlueprintCallable, BlueprintPure=false)
    TArray<UUR_InventoryItemInstance*> GetSlots() const
    {
        return Slots;
    }

    UFUNCTION(BlueprintCallable, BlueprintPure=false)
    int32 GetActiveSlotIndex() const
    {
        return ActiveSlotIndex;
    }

    UFUNCTION(BlueprintCallable, BlueprintPure = false)
    UUR_InventoryItemInstance* GetActiveSlotItem() const;

    UFUNCTION(BlueprintCallable, BlueprintPure=false)
    int32 GetNextFreeItemSlot() const;

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
    void AddItemToSlot(int32 SlotIndex, UUR_InventoryItemInstance* Item);

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
    UUR_InventoryItemInstance* RemoveItemFromSlot(int32 SlotIndex);

    virtual void BeginPlay() override;

private:
    void UnequipItemInSlot();
    void EquipItemInSlot();

    UUR_EquipmentManagerComponent* FindEquipmentManager() const;

protected:
    UPROPERTY()
    int32 NumSlots;

    UFUNCTION()
    void OnRep_Slots();

    UFUNCTION()
    void OnRep_ActiveSlotIndex();

private:
    UPROPERTY(ReplicatedUsing=OnRep_Slots)
    TArray<TObjectPtr<UUR_InventoryItemInstance>> Slots;

    UPROPERTY(ReplicatedUsing=OnRep_ActiveSlotIndex)
    int32 ActiveSlotIndex = -1;

    UPROPERTY()
    TObjectPtr<UUR_EquipmentInstance> EquippedItem;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FGameQuickBarSlotsChangedMessage
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category=Inventory)
    TObjectPtr<AActor> Owner = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = Inventory)
    TArray<TObjectPtr<UUR_InventoryItemInstance>> Slots;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FGameQuickBarActiveIndexChangedMessage
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category=Inventory)
    TObjectPtr<AActor> Owner = nullptr;

    UPROPERTY(BlueprintReadOnly, Category=Inventory)
    int32 ActiveIndex = 0;
};
