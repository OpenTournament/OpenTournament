// Copyright 2019-2020 Open Tournament Project, All Rights Reserved.

#include "UR_ArmorComponent.h"

#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UUR_ArmorComponent::UUR_ArmorComponent()
    : Armor(100)
    , ArmorMax(200)
    //, hasBarrier(false)
{
    SetIsReplicatedByDefault(true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_ArmorComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    DOREPLIFETIME(UUR_ArmorComponent, Armor);
    DOREPLIFETIME(UUR_ArmorComponent, ArmorMax);
    DOREPLIFETIME(UUR_ArmorComponent, hasBarrier);
}


// Called when the game starts
/*void UUR_ArmorComponent::BeginPlay()
{
    Super::BeginPlay();

    // ...
    
}


// Called every frame
void UUR_ArmorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ...
}*/

void UUR_ArmorComponent::SetArmor(const int32 InValue)
{
    Armor = InValue;
}

void UUR_ArmorComponent::ChangeArmor(const int32 InChangeValue)
{
    Armor += InChangeValue;
}

void UUR_ArmorComponent::SetBarrier(bool barrier)
{
    hasBarrier = barrier;
}
