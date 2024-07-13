// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SubclassOf.h"
#include "UObject/Interface.h"

#include "UObject/ObjectPtr.h"
#include "IPickupable.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

template <typename InterfaceType>
class TScriptInterface;

class AActor;
class UUR_InventoryItemDefinition;
class UUR_InventoryItemInstance;
class UUR_InventoryManagerComponent;
class UObject;
struct FFrame;

/////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FPickupTemplate
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere)
    int32 StackCount = 1;

    UPROPERTY(EditAnywhere)
    TSubclassOf<UUR_InventoryItemDefinition> ItemDef;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FPickupInstance
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UUR_InventoryItemInstance> Item = nullptr;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FInventoryPickup
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FPickupInstance> Instances;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FPickupTemplate> Templates;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

/**  */
UINTERFACE(MinimalAPI, BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UPickupable : public UInterface
{
    GENERATED_BODY()
};

/**  */
class IPickupable
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    virtual FInventoryPickup GetPickupInventory() const = 0;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

/**  */
UCLASS()
class UPickupableStatics : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UPickupableStatics();

public:
    UFUNCTION(BlueprintPure)
    static TScriptInterface<IPickupable> GetFirstPickupableFromActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, meta = (WorldContext = "Ability"))
    static void AddPickupToInventory(UUR_InventoryManagerComponent* InventoryComponent, TScriptInterface<IPickupable> Pickup);
};
