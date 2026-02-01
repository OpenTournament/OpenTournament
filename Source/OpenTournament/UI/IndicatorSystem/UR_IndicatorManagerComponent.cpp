// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_IndicatorManagerComponent.h"

#include "IndicatorDescriptor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_IndicatorManagerComponent)

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_IndicatorManagerComponent::UUR_IndicatorManagerComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bAutoRegister = true;
    bAutoActivate = true;
}

/*static*/
UUR_IndicatorManagerComponent* UUR_IndicatorManagerComponent::GetComponent(AController* Controller)
{
    if (Controller)
    {
        return Controller->FindComponentByClass<UUR_IndicatorManagerComponent>();
    }

    return nullptr;
}

void UUR_IndicatorManagerComponent::AddIndicator(UIndicatorDescriptor* IndicatorDescriptor)
{
    IndicatorDescriptor->SetIndicatorManagerComponent(this);
    OnIndicatorAdded.Broadcast(IndicatorDescriptor);
    Indicators.Add(IndicatorDescriptor);
}

void UUR_IndicatorManagerComponent::RemoveIndicator(UIndicatorDescriptor* IndicatorDescriptor)
{
    if (IndicatorDescriptor)
    {
        ensure(IndicatorDescriptor->GetIndicatorManagerComponent() == this);

        OnIndicatorRemoved.Broadcast(IndicatorDescriptor);
        Indicators.Remove(IndicatorDescriptor);
    }
}
