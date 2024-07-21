// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "AbilitySystemGlobals.h"

#include "UR_AbilitySystemGlobals.generated.h"

class UObject;
struct FGameplayEffectContext;

UCLASS(Config=Game)
class UUR_AbilitySystemGlobals : public UAbilitySystemGlobals
{
    GENERATED_UCLASS_BODY()
    //~UAbilitySystemGlobals interface
    virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;

    //~End of UAbilitySystemGlobals interface
};
