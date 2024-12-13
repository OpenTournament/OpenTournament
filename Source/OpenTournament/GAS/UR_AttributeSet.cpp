// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_AttributeSet.h"
#include "UR_AbilitySystemComponent.h"
#include "UR_LogChannels.h"
#include "UR_GameplayTags.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_AttributeSet)

UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_Armor_Change, "Gameplay.Armor.Change");
UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_MaxArmor_Change, "Gameplay.MaxArmor.Change");

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_AttributeSet::UUR_AttributeSet()
    : OverHealth(0.f)
    , OverHealthMax(99.f)
    , Armor(0.0f)
    , ArmorMax(100.0f)
    , ArmorAbsorptionPercent(0.66f)
    , Shield(0.0f)
    , ShieldMax(100.0f)
    , Energy(0.f)
    , EnergyMax(100.f)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_AttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, OverHealth);
    DOREPLIFETIME(ThisClass, OverHealthMax);
    DOREPLIFETIME(ThisClass, Energy);
    DOREPLIFETIME(ThisClass, EnergyMax);
    DOREPLIFETIME(ThisClass, Armor);
    DOREPLIFETIME(ThisClass, ArmorMax);
    DOREPLIFETIME(ThisClass, ArmorAbsorptionPercent);
    DOREPLIFETIME(ThisClass, Shield);
    DOREPLIFETIME(ThisClass, ShieldMax);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_AttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    // This is called whenever attributes change, so for max health (etc) we want to scale the current totals to match
    Super::PreAttributeChange(Attribute, NewValue);

    if (Attribute == GetOverHealthAttribute())
    {
        OverHealth.SetCurrentValue(FMath::Clamp(NewValue, 0.f, OverHealthMax.GetCurrentValue()));
    }
    else if (Attribute == GetArmorAttribute())
    {
        Armor.SetCurrentValue(FMath::Clamp(NewValue, 0.f, ArmorMax.GetCurrentValue()));
    }
    else if (Attribute == GetShieldAttribute())
    {
        Shield.SetCurrentValue(FMath::Clamp(NewValue, 0.f, ShieldMax.GetCurrentValue()));
    }
    else if (Attribute == GetEnergyAttribute())
    {
        Energy.SetCurrentValue(FMath::Clamp(NewValue, 0.f, EnergyMax.GetCurrentValue()));
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
        const float NewDelta = (CurrentMaxValue > 0.f) ? (CurrentValue * NewMaxValue / CurrentMaxValue) - CurrentValue : NewMaxValue;

        AbilityComponent->ApplyModToAttributeUnsafe(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
    }
}

void UUR_AttributeSet::OnRep_OverHealth(const FGameplayAttributeData& OldOverHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UUR_AttributeSet, OverHealth, OldOverHealth);
}

void UUR_AttributeSet::OnRep_OverHealthMax(const FGameplayAttributeData& OldOverHealthMax)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UUR_AttributeSet, OverHealthMax, OldOverHealthMax);
}

void UUR_AttributeSet::OnRep_Energy(const FGameplayAttributeData& OldEnergy)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UUR_AttributeSet, Energy, OldEnergy);
}

void UUR_AttributeSet::OnRep_EnergyMax(const FGameplayAttributeData& OldEnergyMax)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UUR_AttributeSet, EnergyMax, OldEnergyMax);
}

void UUR_AttributeSet::OnRep_Armor(const FGameplayAttributeData& OldArmor)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UUR_AttributeSet, Armor, OldArmor);
}

void UUR_AttributeSet::OnRep_ArmorMax(const FGameplayAttributeData& OldArmorMax)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UUR_AttributeSet, ArmorMax, OldArmorMax);
}

void UUR_AttributeSet::OnRep_ArmorAbsorptionPercent(const FGameplayAttributeData& OldArmorAbsorptionPercent)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UUR_AttributeSet, ArmorAbsorptionPercent, OldArmorAbsorptionPercent);
}

void UUR_AttributeSet::OnRep_Shield(const FGameplayAttributeData& OldShield)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UUR_AttributeSet, Shield, OldShield);
}

void UUR_AttributeSet::OnRep_ShieldMax(const FGameplayAttributeData& OldShieldMax)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UUR_AttributeSet, ShieldMax, OldShieldMax);
}

float UUR_AttributeSet::GetEffectiveHealth()
{
    const float TotalHealth = 0 + GetOverHealth();

    if (GetArmorAbsorptionPercent() < 1)
    {
        return GetShield() + FMath::Min(TotalHealth + GetArmor(), TotalHealth / (1.f - GetArmorAbsorptionPercent()));
    }

    return GetShield() + GetArmor() + TotalHealth;
}

UUR_AbilitySystemComponent* UUR_AttributeSet::GetGameAbilitySystemComponent() const
{
    return Cast<UUR_AbilitySystemComponent>(GetOwningAbilitySystemComponent());
}
