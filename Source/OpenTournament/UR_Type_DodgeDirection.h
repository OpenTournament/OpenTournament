// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UR_Type_DodgeDirection.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Enumeration for Dodge Directions used by Character Movement & Input
 */
UENUM(BlueprintType)
enum class EDodgeDirection : uint8
{
    None,
    Forward 	 UMETA(DisplayName = "DodgeForward"),
    Backward 	 UMETA(DisplayName = "DodgeBackward"),
    Left	     UMETA(DisplayName = "DodgeLeft"),
    Right        UMETA(DisplayName = "DodgeRight"),
    Up           UMETA(DisplayName = "DodgeUp"),
    Down         UMETA(DisplayName = "DodgeDown")
};
