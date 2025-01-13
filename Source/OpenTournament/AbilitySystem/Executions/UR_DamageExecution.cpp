// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_DamageExecution.h"

#include "Engine/World.h"

#include "GAS/UR_AbilitySourceInterface.h"
#include "GAS/UR_GameplayEffectContext.h"
#include "AbilitySystem/Attributes/UR_CombatSet.h"
#include "AbilitySystem/Attributes/UR_HealthSet.h"
//#include "Teams/UR_TeamSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_DamageExecution)

/////////////////////////////////////////////////////////////////////////////////////////////////

struct FDamageStatics
{
    FGameplayEffectAttributeCaptureDefinition BaseDamageDef;

    FDamageStatics()
    {
        BaseDamageDef = FGameplayEffectAttributeCaptureDefinition(UUR_CombatSet::GetBaseDamageAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
    }
};

static FDamageStatics& DamageStatics()
{
    static FDamageStatics Statics;
    return Statics;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_DamageExecution::UUR_DamageExecution()
{
    RelevantAttributesToCapture.Add(DamageStatics().BaseDamageDef);
}

void UUR_DamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
#if WITH_SERVER_CODE
    const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
    FUR_GameplayEffectContext* TypedContext = FUR_GameplayEffectContext::ExtractEffectContext(Spec.GetContext());
    check(TypedContext);

    const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

    FAggregatorEvaluateParameters EvaluateParameters;
    EvaluateParameters.SourceTags = SourceTags;
    EvaluateParameters.TargetTags = TargetTags;

    float BaseDamage = 0.0f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BaseDamageDef, EvaluateParameters, BaseDamage);

    const AActor* EffectCauser = TypedContext->GetEffectCauser();
    const FHitResult* HitActorResult = TypedContext->GetHitResult();

    AActor* HitActor = nullptr;
    FVector ImpactLocation = FVector::ZeroVector;
    FVector ImpactNormal = FVector::ZeroVector;
    FVector StartTrace = FVector::ZeroVector;
    FVector EndTrace = FVector::ZeroVector;

    // Calculation of hit actor, surface, zone, and distance all rely on whether the calculation has a hit result or not.
    // Effects just being added directly w/o having been targeted will always come in without a hit result, which must default
    // to some fallback information.
    if (HitActorResult)
    {
        const FHitResult& CurHitResult = *HitActorResult;
        HitActor = CurHitResult.HitObjectHandle.FetchActor();
        if (HitActor)
        {
            ImpactLocation = CurHitResult.ImpactPoint;
            ImpactNormal = CurHitResult.ImpactNormal;
            StartTrace = CurHitResult.TraceStart;
            EndTrace = CurHitResult.TraceEnd;
        }
    }

    // Handle case of no hit result or hit result not actually returning an actor
    UAbilitySystemComponent* TargetAbilitySystemComponent = ExecutionParams.GetTargetAbilitySystemComponent();
    if (!HitActor)
    {
        HitActor = TargetAbilitySystemComponent ? TargetAbilitySystemComponent->GetAvatarActor_Direct() : nullptr;
        if (HitActor)
        {
            ImpactLocation = HitActor->GetActorLocation();
        }
    }

    // Apply rules for team damage/self damage/etc...
    float DamageInteractionAllowedMultiplier = 1.0f;
    if (HitActor)
    {
        // @! TODO Add TeamSubsystem
        //UUR_TeamSubsystem* TeamSubsystem = HitActor->GetWorld()->GetSubsystem<UUR_TeamSubsystem>();
        //if (ensure(TeamSubsystem))
        //{
        //    DamageInteractionAllowedMultiplier = TeamSubsystem->CanCauseDamage(EffectCauser, HitActor) ? 1.0 : 0.0;
        //}
    }

    // Determine distance
    double Distance = WORLD_MAX;

    if (TypedContext->HasOrigin())
    {
        Distance = FVector::Dist(TypedContext->GetOrigin(), ImpactLocation);
    }
    else if (EffectCauser)
    {
        Distance = FVector::Dist(EffectCauser->GetActorLocation(), ImpactLocation);
    }
    else
    {
        ensureMsgf(false, TEXT("Damage Calculation cannot deduce a source location for damage coming from %s; Falling back to WORLD_MAX dist!"), *GetPathNameSafe(Spec.Def));
    }

    // Apply ability source modifiers
    float PhysicalMaterialAttenuation = 1.0f;
    float DistanceAttenuation = 1.0f;
    if (const IUR_AbilitySourceInterface* AbilitySource = TypedContext->GetAbilitySource())
    {
        if (const UPhysicalMaterial* PhysMat = TypedContext->GetPhysicalMaterial())
        {
            PhysicalMaterialAttenuation = AbilitySource->GetPhysicalMaterialAttenuation(PhysMat, SourceTags, TargetTags);
        }

        DistanceAttenuation = AbilitySource->GetDistanceAttenuation(Distance, SourceTags, TargetTags);
    }
    DistanceAttenuation = FMath::Max(DistanceAttenuation, 0.0f);

    // Clamping is done when damage is converted to -health
    const float DamageDone = FMath::Max(BaseDamage * DistanceAttenuation * PhysicalMaterialAttenuation * DamageInteractionAllowedMultiplier, 0.0f);

    if (DamageDone > 0.0f)
    {
        // Apply a damage modifier, this gets turned into - health on the target
        OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UUR_HealthSet::GetDamageAttribute(), EGameplayModOp::Additive, DamageDone));
    }
#endif // #if WITH_SERVER_CODE
}
