// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Engine/LocalPlayer.h"
#include "UR_LocalPlayer.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UUR_MessageHistory;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * 
 */
UCLASS(BlueprintType)
class OPENTOURNAMENT_API UUR_LocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()

	UUR_LocalPlayer(const FObjectInitializer& ObjectInitializer);

public:

	/**
	* Reference to message history subobject.
	*/
	UPROPERTY(BlueprintReadOnly)
	UUR_MessageHistory* MessageHistory;
};
