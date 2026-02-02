// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_TargetType.h"
#include "UR_GameplayAbility.h"
#include "UR_Character.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_TargetType)

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_TargetType::GetTargets_Implementation(AUR_Character* TargetingCharacter, AActor* TargetingActor, FGameplayEventData EventData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const
{
    return;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_TargetType_UseOwner::GetTargets_Implementation(AUR_Character* TargetingCharacter, AActor* TargetingActor, FGameplayEventData EventData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const
{
    OutActors.Add(TargetingCharacter);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_TargetType_UseEventData::GetTargets_Implementation(AUR_Character* TargetingCharacter, AActor* TargetingActor, FGameplayEventData EventData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const
{
    const FHitResult* FoundHitResult = EventData.ContextHandle.GetHitResult();
    if (FoundHitResult)
    {
        OutHitResults.Add(*FoundHitResult);
    }
    else if (EventData.Target)
    {
        OutActors.Add(const_cast<AActor*>(EventData.Target.Get()));
    }
}
