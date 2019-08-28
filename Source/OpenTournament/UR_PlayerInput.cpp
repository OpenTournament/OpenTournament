// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_PlayerInput.h"

void UUR_PlayerInput::SayHello()
{
	UE_LOG(LogTemp, Warning, TEXT("UR_PlayerInput says Hello."));
}

/*
void UUR_PlayerInput::SetActionKeyMappingKey(FInputActionKeyMapping & ActionKeyMapping, FKey Key)
{
	ActionKeyMapping.Key = Key;
}
*/

void UUR_PlayerInput::SetActionKeyMappingKey(const FInputActionKeyMapping ActionKeyMapping, FKey Key)
{
	int FoundIndex = -1;
	if (ActionMappings.Find(ActionKeyMapping, FoundIndex))
	{
		FInputActionKeyMapping & KeyMappingToModify = ActionMappings[FoundIndex];
		KeyMappingToModify.Key = Key;
		ForceRebuildingKeyMaps(false);
		//SaveConfig();
	}
	else
		return;
}
