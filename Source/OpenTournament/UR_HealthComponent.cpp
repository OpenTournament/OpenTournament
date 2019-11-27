// Copyright 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_HealthComponent.h"

#include "UnrealNetwork.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_HealthComponent::UUR_HealthComponent()
    : Health(100)
    , HealthMax(100)
    , SuperHealthMax(200)
{
    bReplicates = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_HealthComponent::GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const
{
    DOREPLIFETIME(UUR_HealthComponent, Health);
    DOREPLIFETIME(UUR_HealthComponent, HealthMax);
    DOREPLIFETIME(UUR_HealthComponent, SuperHealthMax);
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

void UUR_HealthComponent::HealBy(const int32 HealAmount, bool bSuperHeal)
{
	int32 NewHealth = FMath::Min(Health + HealAmount, bSuperHeal ? SuperHealthMax : HealthMax);
	Health = FMath::Max(Health, NewHealth);
}
