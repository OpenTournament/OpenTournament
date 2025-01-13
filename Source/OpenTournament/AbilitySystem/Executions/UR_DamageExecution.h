// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameplayEffectExecutionCalculation.h"

#include "UR_DamageExecution.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * UUR_DamageExecution
 *
 *	Execution used by gameplay effects to apply damage to the health attributes.
 */
UCLASS()
class UUR_DamageExecution : public UGameplayEffectExecutionCalculation
{
    GENERATED_BODY()

public:
    UUR_DamageExecution();

protected:
    virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
