// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "UR_InventoryItemDefinition.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
class TSubclassOf;

class UUR_InventoryItemInstance;
struct FFrame;

/////////////////////////////////////////////////////////////////////////////////////////////////

// Represents a fragment of an item definition
UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class OPENTOURNAMENT_API UUR_InventoryItemFragment : public UObject
{
    GENERATED_BODY()

public:
    virtual void OnInstanceCreated(UUR_InventoryItemInstance* Instance) const
    {
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * UUR_InventoryItemDefinition
 */
UCLASS(Blueprintable, Const, Abstract)
class UUR_InventoryItemDefinition : public UObject
{
    GENERATED_BODY()

public:
    UUR_InventoryItemDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display)
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display, Instanced)
    TArray<TObjectPtr<UUR_InventoryItemFragment>> Fragments;

public:
    const UUR_InventoryItemFragment* FindFragmentByClass(TSubclassOf<UUR_InventoryItemFragment> FragmentClass) const;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

//@TODO: Make into a subsystem instead?
UCLASS()
class UUR_InventoryFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

    UFUNCTION(BlueprintCallable, meta=(DeterminesOutputType=FragmentClass))
    static const UUR_InventoryItemFragment* FindItemDefinitionFragment(TSubclassOf<UUR_InventoryItemDefinition> ItemDef, TSubclassOf<UUR_InventoryItemFragment> FragmentClass);
};
