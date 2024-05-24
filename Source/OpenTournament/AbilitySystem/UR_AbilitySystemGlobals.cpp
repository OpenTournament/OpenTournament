// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_AbilitySystemGlobals.h"

#include "UR_GameplayEffectContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_AbilitySystemGlobals)

struct FGameplayEffectContext;

UUR_AbilitySystemGlobals::UUR_AbilitySystemGlobals(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

FGameplayEffectContext* UUR_AbilitySystemGlobals::AllocGameplayEffectContext() const
{
    return new FUR_GameplayEffectContext();
}
