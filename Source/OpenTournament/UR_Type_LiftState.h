// Copyright 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"

#include "UR_Type_LiftState.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Enumeration for Dodge Directions used by Character Movement & Input
 */
UENUM()
enum class ELiftState : uint8
{
	LS_Start		UMETA(DisplayName = "Start"),
	LS_Moving		UMETA(DisplayName = "Moving"),
	LS_End			UMETA(DisplayName = "End"),
};