// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UObject/Interface.h"

#include "UR_AbilitySourceInterface.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UObject;
class UPhysicalMaterial;
struct FGameplayTagContainer;

/////////////////////////////////////////////////////////////////////////////////////////////////

/** Base interface for anything acting as a ability calculation source */
UINTERFACE()
class UUR_AbilitySourceInterface : public UInterface
{
    GENERATED_UINTERFACE_BODY()
};

class IUR_AbilitySourceInterface
{
    GENERATED_IINTERFACE_BODY()

    /**
     * Compute the multiplier for effect falloff with distance
     *
     * @param Distance			Distance from source to target for ability calculations (distance bullet traveled for a gun, etc...)
     * @param SourceTags		Aggregated Tags from the source
     * @param TargetTags		Aggregated Tags currently on the target
     *
     * @return Multiplier to apply to the base attribute value due to distance
     */
    virtual float GetDistanceAttenuation(float Distance, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr) const = 0;

    virtual float GetPhysicalMaterialAttenuation(const UPhysicalMaterial* PhysicalMaterial, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr) const = 0;
};
