// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_CombatSet.h"

#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_CombatSet)

/////////////////////////////////////////////////////////////////////////////////////////////////

class FLifetimeProperty;

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_CombatSet::UUR_CombatSet()
    : BaseDamage(0.0f)
    , BaseHeal(0.0f)
{
    // Noop
}

void UUR_CombatSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(UUR_CombatSet, BaseDamage, COND_OwnerOnly, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UUR_CombatSet, BaseHeal, COND_OwnerOnly, REPNOTIFY_Always);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_CombatSet::OnRep_BaseDamage(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UUR_CombatSet, BaseDamage, OldValue);
}

void UUR_CombatSet::OnRep_BaseHeal(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UUR_CombatSet, BaseHeal, OldValue);
}
