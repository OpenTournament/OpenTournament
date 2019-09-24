// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
