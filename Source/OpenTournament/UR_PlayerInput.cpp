// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_PlayerInput.h"

#include "GameFramework/InputSettings.h"

#include "Engine/World.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_PlayerInput::PostInitProperties()
{
    Super::PostInitProperties();

    if (GWorld && GWorld->IsGameWorld())
    {
        SetupUserSettings();
    }
}

void UUR_PlayerInput::SetupUserSettings()
{
    int32 SavedActionsNum = UserActionMappings.Num();
    int32 SavedAxisNum = UserAxisMappings.Num();
    int32 SavedAxisConfigNum = UserAxisConfigs.Num();

    // Project defaults
    const UInputSettings* InputSettings = UInputSettings::GetInputSettings();

    // Merge any default action missing in user config
    TArray<FInputActionKeyMapping> DefaultActionMappings = InputSettings->GetActionMappings();
    TArray<FInputActionKeyMapping> FoundActions;
    for (const auto& DefAction : DefaultActionMappings)
    {
        if (!FindUserActionMappings(DefAction.ActionName, FoundActions))
        {
            UserActionMappings.Add(DefAction);
        }
    }

    // Merge any default axis missing in user config
    TArray<FInputAxisKeyMapping> DefaultAxisMappings = InputSettings->GetAxisMappings();
    TArray<FInputAxisKeyMapping> FoundAxis;
    for (const auto& DefAxis : DefaultAxisMappings)
    {
        if (!FindUserAxisMappings(DefAxis.AxisName, FoundAxis))
        {
            UserAxisMappings.Add(DefAxis);
        }
    }

    FInputAxisConfigEntry FoundAxisConfig;
    for (const auto& DefAxisConfig : InputSettings->AxisConfig)
    {
        if (!FindUserAxisConfig(DefAxisConfig.AxisKeyName, FoundAxisConfig))
        {
            UserAxisConfigs.Add(DefAxisConfig);
        }
    }

    // Take out Tap* actions, we don't want them in config, they are regenerated dynamically.
    // The only reason we need them in project settings, is to expose Tap* events to Blueprints.
    FName TapActionName;
    for (const auto& AxisMapping : UserAxisMappings)
    {
        if (AxisShouldGenerateTapAction(AxisMapping.AxisName, TapActionName))
        {
            UserActionMappings.RemoveAll([TapActionName](const FInputActionKeyMapping& Other) {
                return Other.ActionName.IsEqual(TapActionName);
            });
        }
    }

    if (UserActionMappings.Num() != SavedActionsNum || UserAxisMappings.Num() != SavedAxisNum || UserAxisConfigs.Num() != SavedAxisConfigNum)
    {
        SaveUserSettings();
    }

    // Activate user keybindings
    RegenerateInternalBindings();
}

void UUR_PlayerInput::SaveUserSettings()
{
    UserActionMappings.Sort();
    UserAxisMappings.Sort();

    SaveConfig();
}

void UUR_PlayerInput::RegenerateInternalBindings()
{
    FName TapActionName;

    AxisMappings = UserAxisMappings;
    ActionMappings = UserActionMappings;
    AxisConfig = UserAxisConfigs;

    // Remove Tap* actions if they are present
    for (const auto& AxisMapping : AxisMappings)
    {
        if (AxisShouldGenerateTapAction(AxisMapping.AxisName, TapActionName))
        {
            ActionMappings.RemoveAll([TapActionName](const FInputActionKeyMapping& Other) {
                return Other.ActionName.IsEqual(TapActionName);
            });
        }
    }

    // Re-add them with proper bindings
    for (const auto& AxisMapping : AxisMappings)
    {
        if (AxisShouldGenerateTapAction(AxisMapping.AxisName, TapActionName))
        {
            ActionMappings.Add(FInputActionKeyMapping(TapActionName, AxisMapping.Key));
        }
    }

    ForceRebuildingKeyMaps(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

bool UUR_PlayerInput::ModifyKeyMapping(const FName& MappingName, const FInputChord& InputChord)
{
    // Get action mappings and axis mappings
    TArray<FInputActionKeyMapping> FoundActionMappings;
    TArray<FInputAxisKeyMapping> FoundAxisMappings;
    FindUserActionMappings(MappingName, FoundActionMappings);
    FindUserAxisMappings(MappingName, FoundAxisMappings);

    // If an action and axis have the same name, print an error and return early
    if (FoundActionMappings.IsValidIndex(0) && FoundAxisMappings.IsValidIndex(0))
    {
        UE_LOG(LogTemp, Error, TEXT("An axis and an action cannot have the same name!"));
        return false;
    }

    // If there isn't an axis or action with the specified name, print an error and return early
    if (!FoundActionMappings.IsValidIndex(0) && !FoundAxisMappings.IsValidIndex(0))
    {
        UE_LOG(LogTemp, Error, TEXT("There isn't an axis or an action defined with that name!"));
        return false;
    }

    // If ActionMappings has at least 1 element, change the key mapping for an action
    if (FoundActionMappings.Num() > 0)
    {
        RemapAction(FoundActionMappings[0], InputChord.Key);
    }

    // Do the same, but for axis mappings
    if (FoundAxisMappings.Num() > 0)
    {
        RemapAxis(FoundAxisMappings[0], InputChord.Key);
    }

    return true;
}

void UUR_PlayerInput::RemapAction(FInputActionKeyMapping& ActionKeyMapping, const FKey& Key)
{
    UserActionMappings.Remove(ActionKeyMapping);
    ActionKeyMapping.Key = Key;
    UserActionMappings.AddUnique(ActionKeyMapping);
    SaveUserSettings();
    RegenerateInternalBindings();
}

void UUR_PlayerInput::RemapAxis(FInputAxisKeyMapping& AxisKeyMapping, const FKey& Key)
{
    UserAxisMappings.Remove(AxisKeyMapping);
    AxisKeyMapping.Key = Key;
    UserAxisMappings.AddUnique(AxisKeyMapping);
    SaveUserSettings();
    RegenerateInternalBindings();
}

/////////////////////////////////////////////////////////////////////////////////////////////////

bool UUR_PlayerInput::FindUserActionMappings(FName ActionName, TArray<FInputActionKeyMapping>& OutMappings) const
{
    OutMappings.Empty();
    for (const auto& UserAction : UserActionMappings)
    {
        if (UserAction.ActionName.IsEqual(ActionName))
        {
            OutMappings.Add(UserAction);
        }
    }
    return OutMappings.Num() > 0;
}

bool UUR_PlayerInput::FindUserAxisMappings(FName AxisName, TArray<FInputAxisKeyMapping>& OutMappings) const
{
    OutMappings.Empty();
    for (const auto& UserAxis : UserAxisMappings)
    {
        if (UserAxis.AxisName.IsEqual(AxisName))
        {
            OutMappings.Add(UserAxis);
        }
    }
    return OutMappings.Num() > 0;
}

bool UUR_PlayerInput::FindUserAxisConfig(FName AxisKeyName, FInputAxisConfigEntry& OutAxisConfig) const
{
    for (const auto& UserAxisConfig : UserAxisConfigs)
    {
        if (UserAxisConfig.AxisKeyName.IsEqual(AxisKeyName))
        {
            OutAxisConfig = UserAxisConfig;
            return true;
        }
    }
    return false;
}

void UUR_PlayerInput::K2_GetUserAxisConfig(FName AxisKeyName, bool& bFound, float& Sensitivity, bool& bInvert)
{
    FInputAxisConfigEntry UserAxisConfig;
    if (FindUserAxisConfig(AxisKeyName, UserAxisConfig))
    {
        bFound = true;
        Sensitivity = UserAxisConfig.AxisProperties.Sensitivity;
        bInvert = UserAxisConfig.AxisProperties.bInvert;
        return;
    }

    bFound = false;
    Sensitivity = 1.f;
    bInvert = false;
}

bool UUR_PlayerInput::K2_UpdateUserAxisConfig(FName AxisKeyName, float Sensitivity, bool bInvert)
{
    for (auto& UserAxisConfig : UserAxisConfigs)
    {
        if (UserAxisConfig.AxisKeyName.IsEqual(AxisKeyName))
        {
            UserAxisConfig.AxisProperties.Sensitivity = Sensitivity;
            UserAxisConfig.AxisProperties.bInvert = bInvert;
            SaveConfig();
            RegenerateInternalBindings();
            return true;
        }
    }
    return false;
}

bool UUR_PlayerInput::AxisShouldGenerateTapAction(FName AxisName, FName& OutTapActionName)
{
    if (AxisName.ToString().Contains(TEXT("Move")))
    {
        OutTapActionName = FName(*AxisName.ToString().Replace(TEXT("Move"), TEXT("Tap")));
        return true;
    }
    OutTapActionName = FName(TEXT(""));
    return false;
}
