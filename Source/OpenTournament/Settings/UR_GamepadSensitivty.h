// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UR_GamepadSensitivty.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

UENUM(BlueprintType)
enum class EGameGamepadSensitivity : uint8
{
    Invalid = 0		UMETA(Hidden),

    Slow			UMETA(DisplayName = "01 - Slow"),
    SlowPlus		UMETA(DisplayName = "02 - Slow+"),
    SlowPlusPlus	UMETA(DisplayName = "03 - Slow++"),
    Normal			UMETA(DisplayName = "04 - Normal"),
    NormalPlus		UMETA(DisplayName = "05 - Normal+"),
    NormalPlusPlus	UMETA(DisplayName = "06 - Normal++"),
    Fast			UMETA(DisplayName = "07 - Fast"),
    FastPlus		UMETA(DisplayName = "08 - Fast+"),
    FastPlusPlus	UMETA(DisplayName = "09 - Fast++"),
    Insane			UMETA(DisplayName = "10 - Insane"),

    MAX				UMETA(Hidden),
};
