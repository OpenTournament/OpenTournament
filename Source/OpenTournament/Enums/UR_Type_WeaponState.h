// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UR_Type_WeaponState.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
UENUM(BlueprintType)
enum class EWeaponState : uint8
{
    /** Weapon is not possessed */
    Dropped,
    /** Weapon is possessed but not equipped */
    Holstered,
    /** Weapon is currently being equipped, will be able to fire soon */
    BringUp,
    /** Weapon is equipped and can start firing anytime */
    Idle,
    /** Weapon is currently firing/charging/on cooldown - a FireMode is active */
    Firing,
    /** Generic state to prevent firing, can be used to implement eg. reloading */
    Busy,
    /** Weapon is currently being unequipped */
    PutDown,

    MAX UMETA(Hidden)
};
