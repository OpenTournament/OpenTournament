// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UR_Type_ExitRotation.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Enumeration for Dodge Directions used by Character Movement & Input
 */
UENUM()
enum class EExitRotation : uint8
{
    Relative	UMETA(DisplayName = "Relative"),
    Fixed 	    UMETA(DisplayName = "Fixed"),
};