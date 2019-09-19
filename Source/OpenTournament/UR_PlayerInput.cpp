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

bool UUR_PlayerInput::ModifyKeyMapping(FName MappingName, const FInputChord InputChord)
{
	UInputSettings * InputSettings = GetDefault<UInputSettings>()->GetInputSettings();
	//Get action mappings and axis mappings
	TArray<FInputActionKeyMapping> ActionMappings;
	TArray<FInputAxisKeyMapping> AxisMappings;
	InputSettings->GetActionMappingByName(MappingName, ActionMappings);
	InputSettings->GetAxisMappingByName(MappingName, AxisMappings);
	//If an action and axis have the same name, print an error and return early
	if (ActionMappings.IsValidIndex(0) && AxisMappings.IsValidIndex(0))
	{
		UE_LOG(LogTemp, Error, TEXT("An axis and an action cannot have the same name!"));
		return false;
	}
	//If there isn't an axis or action with the specified name, print an error and return early
	if (!ActionMappings.IsValidIndex(0) && !AxisMappings.IsValidIndex(0))
	{
		UE_LOG(LogTemp, Error, TEXT("There isn't an axis or an action defined with that name!"));
		return false;
	}
	//If ActionMappings has at least 1 element, change the key mapping for an action
	if (ActionMappings.IsValidIndex(0))
	{
		InputSettings->RemoveActionMapping(ActionMappings[0], false);
		ActionMappings[0].Key = InputChord.Key;
		InputSettings->AddActionMapping(ActionMappings[0], true);
	}
	//Do the same, but for axis mappings
	if (AxisMappings.IsValidIndex(0))
	{
		InputSettings->RemoveAxisMapping(AxisMappings[0], false);
		AxisMappings[0].Key = InputChord.Key;
		InputSettings->AddAxisMapping(AxisMappings[0], true);
	}

	InputSettings->SaveKeyMappings();
	return true;
}

void UUR_PlayerInput::SaveMappings()
{
	/*UInputSettings * InputSettings = GetMutableDefault<UInputSettings>();
	InputSettings->ActionMappings = ActionMappings;
	InputSettings->SaveKeyMappings();*/

	PlayerActionMappings = ActionMappings;
	SaveConfig();
}
