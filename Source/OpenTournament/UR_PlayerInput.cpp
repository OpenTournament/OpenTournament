// Copyright 2019-2020 Open Tournament Project, All Rights Reserved.


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

//is this used ? seems redundant with RemapAction
void UUR_PlayerInput::SetActionKeyMappingKey(const FInputActionKeyMapping& ActionKeyMapping, const FKey& Key)
{
	int32 FoundIndex = -1;
	if (ActionMappings.Find(ActionKeyMapping, FoundIndex))
	{
		FInputActionKeyMapping & KeyMappingToModify = ActionMappings[FoundIndex];
		KeyMappingToModify.Key = Key;
		ForceRebuildingKeyMaps(false);
	}
	else
		return;
}

void UUR_PlayerInput::ModifyActionKeyMapping(const FName& ActionName, const FInputActionKeyMapping& ModActionKeyMapping)
{
    TArray<FInputActionKeyMapping> FoundActionMappings;
	InputSettings->GetActionMappingByName(ActionName, FoundActionMappings);

	InputSettings->RemoveActionMapping(FoundActionMappings[0], false);
	InputSettings->AddActionMapping(ModActionKeyMapping, true);

	InputSettings->SaveKeyMappings();
}

bool UUR_PlayerInput::ModifyKeyMapping(const FName& MappingName, const FInputChord& InputChord)
{
	//Get action mappings and axis mappings
    TArray<FInputActionKeyMapping> FoundActionMappings;
    TArray<FInputAxisKeyMapping> FoundAxisMappings;
	InputSettings->GetActionMappingByName(MappingName, FoundActionMappings);
	InputSettings->GetAxisMappingByName(MappingName, FoundAxisMappings);
	//If an action and axis have the same name, print an error and return early
	if (FoundActionMappings.IsValidIndex(0) && FoundAxisMappings.IsValidIndex(0))
	{
		UE_LOG(LogTemp, Error, TEXT("An axis and an action cannot have the same name (%s)"), *MappingName.ToString());
		return false;
	}
	//If there isn't an axis or action with the specified name, print an error and return early
	if (!FoundActionMappings.IsValidIndex(0) && !FoundAxisMappings.IsValidIndex(0))
	{
		UE_LOG(LogTemp, Error, TEXT("There isn't an axis or an action defined with that name (%s)"), *MappingName.ToString());
		return false;
	}
	//If ActionMappings has at least 1 element, change the key mapping for an action
	if (FoundActionMappings.Num() > 0)
	{
		RemapAction(FoundActionMappings[0], InputChord.Key);
	}
	//Do the same, but for axis mappings
	if (FoundAxisMappings.Num() > 0)
	{
		RemapAxis(FoundAxisMappings[0], InputChord.Key);
	}

	InputSettings->SaveKeyMappings();
	return true;
}

void UUR_PlayerInput::PostInitProperties()
{
	Super::PostInitProperties();
	InputSettings = GetDefault<UInputSettings>()->GetInputSettings();
}

void UUR_PlayerInput::SaveMappings()
{
	/*UInputSettings * InputSettings = GetMutableDefault<UInputSettings>();
	InputSettings->ActionMappings = ActionMappings;
	InputSettings->SaveKeyMappings();*/

	PlayerActionMappings = ActionMappings;
	SaveConfig();
}

/*
Remap the given action to the given key
*/
void UUR_PlayerInput::RemapAction(FInputActionKeyMapping ActionKeyMapping, const FKey& Key)
{
	InputSettings->RemoveActionMapping(ActionKeyMapping, false);
	ActionKeyMapping.Key = Key;
	InputSettings->AddActionMapping(ActionKeyMapping, true);
}

/*
Remap the given axis to the given key
This will also map a corresponding Tap action to the given axis (ex: MoveForward and TapForward)
*/
void UUR_PlayerInput::RemapAxis(FInputAxisKeyMapping AxisKeyMapping, const FKey& Key)
{
	FString AxisString = AxisKeyMapping.AxisName.ToString();
	FString TapActionString = AxisString.Replace(TEXT("Move"), TEXT("Tap"));
	FInputActionKeyMapping TapActionMapping = FInputActionKeyMapping(FName(*TapActionString), AxisKeyMapping.Key);
	InputSettings->RemoveAxisMapping(AxisKeyMapping, false);
	InputSettings->RemoveActionMapping(TapActionMapping, false);
	AxisKeyMapping.Key = Key;
	TapActionMapping.Key = Key;
	InputSettings->AddActionMapping(TapActionMapping, false);
	InputSettings->AddAxisMapping(AxisKeyMapping, true);
}
