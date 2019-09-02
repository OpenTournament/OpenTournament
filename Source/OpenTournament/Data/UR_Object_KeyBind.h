// Fill out your copyright notice in the Description page of Project Settings.

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

	FName ActionName;

	FKey Key;
};
