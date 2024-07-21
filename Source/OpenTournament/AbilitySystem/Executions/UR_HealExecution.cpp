// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_HealExecution.h"
#include "AbilitySystem/Attributes/UR_CombatSet.h"
#include "AbilitySystem/Attributes/UR_HealthSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_HealExecution)

/////////////////////////////////////////////////////////////////////////////////////////////////

struct FHealStatics
{
    FGameplayEffectAttributeCaptureDefinition BaseHealDef;

    FHealStatics()
    {
        BaseHealDef = FGameplayEffectAttributeCaptureDefinition(UUR_CombatSet::GetBaseHealAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
    }
};

static FHealStatics& HealStatics()
{
    static FHealStatics Statics;
    return Statics;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_HealExecution::UUR_HealExecution()
{
    RelevantAttributesToCapture.Add(HealStatics().BaseHealDef);
}

void UUR_HealExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
#if WITH_SERVER_CODE
    const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

    const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

    FAggregatorEvaluateParameters EvaluateParameters;
    EvaluateParameters.SourceTags = SourceTags;
    EvaluateParameters.TargetTags = TargetTags;

    float BaseHeal = 0.0f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(HealStatics().BaseHealDef, EvaluateParameters, BaseHeal);

    const float HealingDone = FMath::Max(0.0f, BaseHeal);

    if (HealingDone > 0.0f)
    {
        // Apply a healing modifier, this gets turned into + health on the target
        OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UUR_HealthSet::GetHealingAttribute(), EGameplayModOp::Additive, HealingDone));
    }
#endif // #if WITH_SERVER_CODE
}
