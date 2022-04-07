// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameFramework/DamageType.h"

#include "UR_DamageType.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UMaterialInterface;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * A DamageType is intended to define and describe a particular form of damage and to provide an avenue
 * for customizing responses to damage from various sources.
 *
 * For example, a game could make a DamageType_Fire set it up to ignite the damaged actor.
 *
 * DamageTypes are never instanced and should be treated as immutable data holders with static code
 * functionality. They should never be stateful.
 */
UCLASS(Blueprintable)
class OPENTOURNAMENT_API UUR_DamageType : public UDamageType
{
    GENERATED_BODY()

public:

    UUR_DamageType()
    {
        PawnOverlayDuration = 0.1f;
    }

    /**
    * Overlay material applied to victim pawn
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInterface* PawnOverlayMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PawnOverlayDuration;

    /**
    * Material used for victim HUD damage indicator widget 
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInterface* HudDamageMaterial;

    /**
    * TBD: To what extent should the damagetype be able to control things?
    *
    * Example, current implementation = character spawns its own overlaymesh and applies DamageType->PawnOverlayMaterial.
    * But another possibility = character calls DamageType->ApplyEffectsTo(self) and let damagetype code do anything.
    *
    * The former factorizes code in a single place but limits possibilities for mod-developpers, they'll have to put handle custom logic in their weapon class.
    * The latter gives custom damagetypes (which are easy to create) more power but increases inheritance and unavoidable code duplication among damagetypes.
    */

};

/////////////////////////////////////////////////////////////////////////////////////////////////
