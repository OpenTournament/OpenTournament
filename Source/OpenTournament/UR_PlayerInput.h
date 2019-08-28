// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerInput.h"
#include "UR_PlayerInput.generated.h"

/**
 * 
 */
UCLASS()
class OPENTOURNAMENT_API UUR_PlayerInput : public UPlayerInput
{
	GENERATED_BODY()
	
public:
	//UPROPERTY(config)
	//TArray<struct FInputActionKeyMapping> ActionMappings;

	void SayHello();

	UFUNCTION(BlueprintCallable, Category="UnrealRemake|UR_PlayerInput")
	void SetActionKeyMappingKey(const FInputActionKeyMapping ActionKeyMapping, FKey Key);
};
