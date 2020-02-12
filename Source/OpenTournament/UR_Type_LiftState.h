// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UR_Type_LiftState.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Enumeration for Dodge Directions used by Character Movement & Input
 */
UENUM()
enum class ELiftState : uint8
{
	Start		UMETA(DisplayName = "Start"),
	Moving		UMETA(DisplayName = "Moving"),
	End			UMETA(DisplayName = "End"),
};