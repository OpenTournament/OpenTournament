// Copyright 2019-2020 Open Tournament Project, All Rights Reserved.

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

	UFUNCTION(BlueprintCallable, Category="UR_PlayerInput")
	void SetActionKeyMappingKey(const FInputActionKeyMapping ActionKeyMapping, FKey Key);

	UFUNCTION(BlueprintCallable, Category = "UR_PlayerInput")
		void ModifyActionKeyMapping(FName ActionName, const FInputActionKeyMapping ModActionKeyMapping);

   /*Modify a key mapping for an action or axis
	returns true if the key mapping was modified succesfully, otherwise returns false
	*/
	bool ModifyKeyMapping(FName MappingName, const FInputChord InputChord);

	void PostInitProperties();

	UFUNCTION(BlueprintCallable, Category = "UR_PlayerInput")
	void SaveMappings();

private:
	UInputSettings * InputSettings;

	void RemapAction(FInputActionKeyMapping ActionKeyMapping, const FKey Key);
	void RemapAxis(FInputAxisKeyMapping AxisKeyMapping, const FKey Key);
};
