// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameplayEffectExecutionCalculation.h"

#include "UR_HealExecution.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UObject;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * UUR_HealExecution
 *
 *	Execution used by gameplay effects to apply healing to the health attributes.
 */
UCLASS()
class UUR_HealExecution : public UGameplayEffectExecutionCalculation
{
    GENERATED_BODY()

public:
    UUR_HealExecution();

protected:
    virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
