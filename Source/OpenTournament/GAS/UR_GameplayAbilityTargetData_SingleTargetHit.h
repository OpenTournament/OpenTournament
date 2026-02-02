// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Abilities/GameplayAbilityTargetTypes.h"

#include "UR_GameplayAbilityTargetData_SingleTargetHit.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class FArchive;
struct FGameplayEffectContextHandle;

/////////////////////////////////////////////////////////////////////////////////////////////////

/** Game-specific additions to SingleTargetHit tracking */
USTRUCT()
struct FUR_GameplayAbilityTargetData_SingleTargetHit : public FGameplayAbilityTargetData_SingleTargetHit
{
    GENERATED_BODY()

    FUR_GameplayAbilityTargetData_SingleTargetHit()
        : CartridgeID(-1)
    {}

    virtual void AddTargetDataToContext(FGameplayEffectContextHandle& Context, bool bIncludeActorArray) const override;

    /** ID to allow the identification of multiple bullets that were part of the same cartridge */
    UPROPERTY()
    int32 CartridgeID;

    // ReSharper disable CppHidingFunction
    bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
    // ReSharper restore CppHidingFunction

    virtual UScriptStruct* GetScriptStruct() const override
    {
        return FUR_GameplayAbilityTargetData_SingleTargetHit::StaticStruct();
    }
};

template <>
struct TStructOpsTypeTraits<FUR_GameplayAbilityTargetData_SingleTargetHit> : public TStructOpsTypeTraitsBase2<FUR_GameplayAbilityTargetData_SingleTargetHit>
{
    enum
    {
        WithNetSerializer = true	// For now this is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
    };
};
