// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UR_Ability_GameActivationPolicy.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * EGameAbilityActivationPolicy
 *
 *	Defines how an ability is meant to activate.
 */
UENUM(BlueprintType)
enum class EGameAbilityActivationPolicy : uint8
{
    // Try to activate the ability when the input is triggered.
    OnInputTriggered,

    // Continually try to activate the ability while the input is active.
    WhileInputActive,

    // Try to activate the ability when an avatar is assigned.
    OnSpawn
};
