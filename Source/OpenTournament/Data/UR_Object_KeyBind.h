// Copyright 2019-2020 Open Tournament Project, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InputCoreTypes.h"
#include "UR_Object_KeyBind.generated.h"

/**
 * 
 */
UCLASS()
class OPENTOURNAMENT_API UUR_Object_KeyBind : public UObject
{
	GENERATED_BODY()
	
public:
	UUR_Object_KeyBind();

	FName Name;

	FKey Key;
};
