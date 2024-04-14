// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_AbilityTypes.h"

#include "AbilitySystemGlobals.h"

#include "OpenTournament.h"
#include "UR_AbilitySystemComponent.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

bool FUR_GameplayEffectContainerSpec::HasValidEffects() const
{
    return TargetGameplayEffectSpecs.Num() > 0;
}

bool FUR_GameplayEffectContainerSpec::HasValidTargets() const
{
    return TargetData.Num() > 0;
}

void FUR_GameplayEffectContainerSpec::AddTargets(const TArray<FHitResult>& HitResults, const TArray<AActor*>& TargetActors)
{
    for (const FHitResult& HitResult : HitResults)
    {
        FGameplayAbilityTargetData_SingleTargetHit* NewData = new FGameplayAbilityTargetData_SingleTargetHit(HitResult);
        TargetData.Add(NewData);
    }

    if (TargetActors.Num() > 0)
    {
        FGameplayAbilityTargetData_ActorArray* NewData = new FGameplayAbilityTargetData_ActorArray();
        NewData->TargetActorArray.Append(TargetActors);
        TargetData.Add(NewData);
    }
}
