// Copyright 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_AttributeSet.h"

#include "UnrealNetwork.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_AttributeSet::UUR_AttributeSet()
  : Health(1.f)
  , HealthMax(1.f)
  , Armor(0.0f)
  , ArmorMax(1.0f)
  , Shield(0.0f)
  , ShieldMax(1.0f)
  , Energy(0.f)
  , EnergyMax(0.f)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_AttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UUR_AttributeSet, Health);
    DOREPLIFETIME(UUR_AttributeSet, HealthMax);
    DOREPLIFETIME(UUR_AttributeSet, Energy);
    DOREPLIFETIME(UUR_AttributeSet, EnergyMax);
    DOREPLIFETIME(UUR_AttributeSet, Armor);
    DOREPLIFETIME(UUR_AttributeSet, ArmorMax);
    DOREPLIFETIME(UUR_AttributeSet, Shield);
    DOREPLIFETIME(UUR_AttributeSet, ShieldMax);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_AttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    // This is called whenever attributes change, so for max health (etc) we want to scale the current totals to match
    Super::PreAttributeChange(Attribute, NewValue);

    if (Attribute == GetHealthMaxAttribute())
    {
        AdjustAttributeForMaxChange(Health, HealthMax, NewValue, GetHealthAttribute());
    }
    else if (Attribute == GetEnergyMaxAttribute())
    {
        AdjustAttributeForMaxChange(Energy, EnergyMax, NewValue, GetEnergyAttribute());
    }
}

void UUR_AttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);
}

void UUR_AttributeSet::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty)
{
    UAbilitySystemComponent* AbilityComponent = GetOwningAbilitySystemComponent();
    const float CurrentMaxValue = MaxAttribute.GetCurrentValue();
    if (!FMath::IsNearlyEqual(CurrentMaxValue, NewMaxValue) && AbilityComponent)
    {
        // Change current value to maintain the current Val / Max percent
        const float CurrentValue = AffectedAttribute.GetCurrentValue();
        float NewDelta = (CurrentMaxValue > 0.f) ? (CurrentValue * NewMaxValue / CurrentMaxValue) - CurrentValue : NewMaxValue;

        AbilityComponent->ApplyModToAttributeUnsafe(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
    }
}

void UUR_AttributeSet::OnRep_Health()
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UUR_AttributeSet, Health);
}

void UUR_AttributeSet::OnRep_HealthMax()
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UUR_AttributeSet, HealthMax);
}

void UUR_AttributeSet::OnRep_Energy()
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UUR_AttributeSet, Energy);
}

void UUR_AttributeSet::OnRep_EnergyMax()
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UUR_AttributeSet, EnergyMax);
}

void UUR_AttributeSet::OnRep_Armor()
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UUR_AttributeSet, Armor);
}

void UUR_AttributeSet::OnRep_ArmorMax()
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UUR_AttributeSet, ArmorMax);
}

void UUR_AttributeSet::OnRep_Shield()
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UUR_AttributeSet, Shield);
}

void UUR_AttributeSet::OnRep_ShieldMax()
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UUR_AttributeSet, ShieldMax);
}