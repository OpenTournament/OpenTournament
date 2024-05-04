// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "AbilitySystemComponent.h"

#include "UR_AttributeSet.h"

#include "UR_CombatSet.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * UUR_CombatSet
 *
 *  Class that defines attributes that are necessary for applying damage or healing.
 *	Attribute examples include: damage, healing, attack power, and shield penetrations.
 */
UCLASS(BlueprintType)
class UUR_CombatSet : public UUR_AttributeSet
{
    GENERATED_BODY()

public:
    UUR_CombatSet();

    ATTRIBUTE_ACCESSORS(UUR_CombatSet, BaseDamage);

    ATTRIBUTE_ACCESSORS(UUR_CombatSet, BaseHeal);

protected:
    UFUNCTION()
    void OnRep_BaseDamage(const FGameplayAttributeData& OldValue);

    UFUNCTION()
    void OnRep_BaseHeal(const FGameplayAttributeData& OldValue);

private:
    // The base amount of damage to apply in the damage execution.
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BaseDamage, Category = "Lyra|Combat", Meta = (AllowPrivateAccess = true))
    FGameplayAttributeData BaseDamage;

    // The base amount of healing to apply in the heal execution.
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BaseHeal, Category = "Lyra|Combat", Meta = (AllowPrivateAccess = true))
    FGameplayAttributeData BaseHeal;
};
