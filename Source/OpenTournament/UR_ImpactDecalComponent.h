// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Components/DecalComponent.h"

#include "UR_ImpactDecalComponent.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UMaterialInterface;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
*
*/
UCLASS(BlueprintType, Blueprintable)
class OPENTOURNAMENT_API UUR_ImpactDecalComponent : public UDecalComponent
{
    GENERATED_BODY()

public:

    UUR_ImpactDecalComponent()
        : CreationTime(0)
    {
        // impact decals should draw on top of level decals
        SortOrder = 3;

        //TODO: impact decals durations should be configurable
        //NOTE: Long fadeout is ugly as fuck, prefer a short quick fadeout at the end of lifetime
        FadeStartDelay = 10.f;
        FadeDuration = 3.f;
    }

    /**
     * The time this component was created, relative to World->GetTimeSeconds().
     * @see UWorld::GetTimeSeconds()
     */
    float CreationTime;

    virtual void BeginPlay() override;

    /**
    * Spawn impact decal.
    * For flat surface decal, Location should be HitLocation and Direction should be -HitNormal.
    */
    UFUNCTION(BlueprintCallable, Meta = (WorldContext = "WorldContext"))
    static UUR_ImpactDecalComponent* SpawnImpactDecal(const UObject* WorldContext, UMaterialInterface* Material, const FVector& Location, const FVector& Direction, FVector Size = FVector(8, 50, 50));

    /**
    * Spawn decal at an angle automatically adjusted between HitNormal and ShotDirection.
    * This gives best results so far to handle corners and stuff, so prefer this.
    * When using this, prefer an uniform box size as the decal will not be placed flat onto surface.
    */
    UFUNCTION(BlueprintCallable, Meta = (WorldContext = "WorldContext"))
    static UUR_ImpactDecalComponent* SpawnImpactDecal2(const UObject* WorldContext, UMaterialInterface* Material, const FVector& HitLocation, const FVector& HitNormal, const FVector& ShotDirection, FVector Size = FVector(50), float MaxAngle = 60);
};
