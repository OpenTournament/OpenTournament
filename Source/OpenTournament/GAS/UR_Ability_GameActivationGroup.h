// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UR_Ability_GameActivationGroup.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * EGameAbilityActivationGroup
 *
 *	Defines how an ability activates in relation to other abilities.
 */
UENUM(BlueprintType)
enum class EGameAbilityActivationGroup : uint8
{
    // Ability runs independently of all other abilities.
    Independent,

    // Ability is canceled and replaced by other exclusive abilities.
    Exclusive_Replaceable,

    // Ability blocks all other exclusive abilities from activating.
    Exclusive_Blocking,

    MAX	UMETA(Hidden)
};
