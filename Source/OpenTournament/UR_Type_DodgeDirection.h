// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UR_Type_DodgeDirection.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Enumeration for Dodge Directions used by Character Movement & Input
 */
UENUM()
enum class EDodgeDirection : uint8
{
    DD_None,
    DD_Forward 	    UMETA(DisplayName = "DodgeForward"),
    DD_Backward 	UMETA(DisplayName = "DodgeBackward"),
    DD_Left	        UMETA(DisplayName = "DodgeLeft"),
    DD_Right        UMETA(DisplayName = "DodgeRight"),
    DD_Up           UMETA(DisplayName = "DodgeUp"),
    DD_Down         UMETA(DisplayName = "DodgeDown")
};