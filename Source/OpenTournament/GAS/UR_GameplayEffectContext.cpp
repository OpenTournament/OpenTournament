// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_GameplayEffectContext.h"

#include "GAS/UR_AbilitySourceInterface.h"
#include "Engine/HitResult.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_GameplayEffectContext)

#if UE_WITH_IRIS
#include "Iris/ReplicationState/PropertyNetSerializerInfoRegistry.h"
#include "Serialization/GameplayEffectContextNetSerializer.h"
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////

class FArchive;

/////////////////////////////////////////////////////////////////////////////////////////////////

FUR_GameplayEffectContext* FUR_GameplayEffectContext::ExtractEffectContext(struct FGameplayEffectContextHandle Handle)
{
    FGameplayEffectContext* BaseEffectContext = Handle.Get();
    if (BaseEffectContext && BaseEffectContext->GetScriptStruct()->IsChildOf(FUR_GameplayEffectContext::StaticStruct()))
    {
        return static_cast<FUR_GameplayEffectContext*>(BaseEffectContext);
    }

    return nullptr;
}

bool FUR_GameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
    FGameplayEffectContext::NetSerialize(Ar, Map, bOutSuccess);

    // Not serialized for post-activation use:
    // CartridgeID

    return true;
}

#if UE_WITH_IRIS
namespace UE::Net
{
    // Forward to FGameplayEffectContextNetSerializer
    // Note: If FUR_GameplayEffectContext::NetSerialize() is modified, a custom NetSerializesr must be implemented as the current fallback will no longer be sufficient.
    UE_NET_IMPLEMENT_FORWARDING_NETSERIALIZER_AND_REGISTRY_DELEGATES(UR_GameplayEffectContext, FGameplayEffectContextNetSerializer);
}
#endif

void FUR_GameplayEffectContext::SetAbilitySource(const IUR_AbilitySourceInterface* InObject, float InSourceLevel)
{
    AbilitySourceObject = MakeWeakObjectPtr(Cast<const UObject>(InObject));
    //SourceLevel = InSourceLevel;
}

const IUR_AbilitySourceInterface* FUR_GameplayEffectContext::GetAbilitySource() const
{
    return Cast<IUR_AbilitySourceInterface>(AbilitySourceObject.Get());
}

const UPhysicalMaterial* FUR_GameplayEffectContext::GetPhysicalMaterial() const
{
    if (const FHitResult* HitResultPtr = GetHitResult())
    {
        return HitResultPtr->PhysMaterial.Get();
    }
    return nullptr;
}
