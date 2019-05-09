// Copyright 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_HealthComponent.h"

#include "UnrealNetwork.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_HealthComponent::UUR_HealthComponent()
    : Health(100.f)
    , HealthMax(200.f)
{
    bReplicates = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_HealthComponent::GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const
{
    DOREPLIFETIME(UUR_HealthComponent, Health);
    DOREPLIFETIME(UUR_HealthComponent, HealthMax);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_HealthComponent::SetHealth(const int32 InValue)
{
    Health = InValue;
}

void UUR_HealthComponent::SetHealthPercentage(const int32 InPercentage)
{
    Health = (HealthMax / 100) * InPercentage;
}

void UUR_HealthComponent::ChangeHealth(const int32 InChangeValue)
{
    Health += InChangeValue;
}

void UUR_HealthComponent::ChangeHealthPercentage(const int32 InChangePercentage)
{
    Health += (HealthMax / 100) * InChangePercentage;
}
