// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerInput.h"
#include "GameFramework/InputSettings.h"
#include "UR_PlayerInput.generated.h"

/**
 * 
 */
UCLASS()
class OPENTOURNAMENT_API UUR_PlayerInput : public UPlayerInput
{
	GENERATED_BODY()
	
public:
	UPROPERTY(config)
	TArray<struct FInputActionKeyMapping> PlayerActionMappings;

	void SayHello();

	void SetupActionMappings();

	UFUNCTION(BlueprintCallable, Category="UnrealRemake|UR_PlayerInput")
	void SetActionKeyMappingKey(const FInputActionKeyMapping ActionKeyMapping, FKey Key);

	UFUNCTION(BlueprintCallable, Category = "UnrealRemake|UR_PlayerInput")
		void ModifyActionKeyMapping(FName ActionName, const FInputActionKeyMapping ModActionKeyMapping);

	UFUNCTION(BlueprintCallable, Category = "UnrealRemake|UR_PlayerInput")
	void SaveMappings();
};
