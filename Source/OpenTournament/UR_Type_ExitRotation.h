// Copyright 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"

#include "UR_Type_ExitRotation.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Enumeration for Dodge Directions used by Character Movement & Input
 */
UENUM()
enum class EExitRotation : uint8
{
	ER_Relative		UMETA(DisplayName = "Relative"),
	ER_Fixed 	    UMETA(DisplayName = "Fixed"),
};