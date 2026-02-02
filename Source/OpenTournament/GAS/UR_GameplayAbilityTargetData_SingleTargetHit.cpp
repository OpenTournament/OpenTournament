// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_GameplayAbilityTargetData_SingleTargetHit.h"

#include "UR_GameplayEffectContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_GameplayAbilityTargetData_SingleTargetHit)

/////////////////////////////////////////////////////////////////////////////////////////////////

struct FGameplayEffectContextHandle;

//////////////////////////////////////////////////////////////////////

void FUR_GameplayAbilityTargetData_SingleTargetHit::AddTargetDataToContext(FGameplayEffectContextHandle& Context, bool bIncludeActorArray) const
{
    FGameplayAbilityTargetData_SingleTargetHit::AddTargetDataToContext(Context, bIncludeActorArray);

    // Add game-specific data
    if (FUR_GameplayEffectContext* TypedContext = FUR_GameplayEffectContext::ExtractEffectContext(Context))
    {
        TypedContext->CartridgeID = CartridgeID;
    }
}

bool FUR_GameplayAbilityTargetData_SingleTargetHit::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
    FGameplayAbilityTargetData_SingleTargetHit::NetSerialize(Ar, Map, bOutSuccess);

    Ar << CartridgeID;

    return true;
}
