// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_PlayerInput.h"

void UUR_PlayerInput::SayHello()
{
	UE_LOG(LogTemp, Warning, TEXT("UR_PlayerInput says Hello."));
}

void UUR_PlayerInput::SetupActionMappings()
{
	//Setting PlayerActionMappings to ActionMappings in case the config is empty
	UE_LOG(LogTemp, Warning, TEXT("Setting up player action mappings"));
	PlayerActionMappings = ActionMappings;
	LoadConfig();
	ActionMappings = PlayerActionMappings;
	ForceRebuildingKeyMaps(false);
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
	}
	else
		return;
}

void UUR_PlayerInput::ModifyActionKeyMapping(FName ActionName, const FInputActionKeyMapping ModActionKeyMapping)
{
	UInputSettings * InputSettings = GetDefault<UInputSettings>()->GetInputSettings();
	TArray<FInputActionKeyMapping> ActionMappings;
	InputSettings->GetActionMappingByName(ActionName, ActionMappings);

	InputSettings->RemoveActionMapping(ActionMappings[0], false);
	InputSettings->AddActionMapping(ModActionKeyMapping, true);

	InputSettings->SaveKeyMappings();
}

void UUR_PlayerInput::SaveMappings()
{
	/*UInputSettings * InputSettings = GetMutableDefault<UInputSettings>();
	InputSettings->ActionMappings = ActionMappings;
	InputSettings->SaveKeyMappings();*/

	PlayerActionMappings = ActionMappings;
	SaveConfig();
}
