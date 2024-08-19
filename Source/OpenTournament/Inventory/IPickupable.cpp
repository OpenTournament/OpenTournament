// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "IPickupable.h"

#include "GameFramework/Actor.h"
#include "UObject/ScriptInterface.h"

#include "UR_InventoryManagerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(IPickupable)

/////////////////////////////////////////////////////////////////////////////////////////////////

class UActorComponent;

/////////////////////////////////////////////////////////////////////////////////////////////////

UPickupableStatics::UPickupableStatics()
    : Super(FObjectInitializer::Get())
{
}

TScriptInterface<IPickupable> UPickupableStatics::GetFirstPickupableFromActor(AActor* Actor)
{
    // If the actor is directly pickupable, return that.
    TScriptInterface<IPickupable> PickupableActor(Actor);
    if (PickupableActor)
    {
        return PickupableActor;
    }

    // If the actor isn't pickupable, it might have a component that has a pickupable interface.
    TArray<UActorComponent*> PickupableComponents = Actor ? Actor->GetComponentsByInterface(UPickupable::StaticClass()) : TArray<UActorComponent*>();
    if (PickupableComponents.Num() > 0)
    {
        // Get first pickupable, if the user needs more sophisticated pickup distinction, will need to be solved elsewhere.
        return TScriptInterface<IPickupable>(PickupableComponents[0]);
    }

    return TScriptInterface<IPickupable>();
}

void UPickupableStatics::AddPickupToInventory(UUR_InventoryManagerComponent* InventoryComponent, TScriptInterface<IPickupable> Pickup)
{
    if (InventoryComponent && Pickup)
    {
        const FInventoryPickup& PickupInventory = Pickup->GetPickupInventory();

        for (const FPickupTemplate& Template : PickupInventory.Templates)
        {
            InventoryComponent->AddItemDefinition(Template.ItemDef, Template.StackCount);
        }

        for (const FPickupInstance& Instance : PickupInventory.Instances)
        {
            InventoryComponent->AddItemInstance(Instance.Item);
        }
    }
}
