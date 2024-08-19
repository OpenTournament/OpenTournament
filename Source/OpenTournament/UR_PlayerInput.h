// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <EnhancedPlayerInput.h>

#include "UR_PlayerInput.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UInputSettings;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
UCLASS(BlueprintType, Config = Input)
class OPENTOURNAMENT_API UUR_PlayerInput
    : public UEnhancedPlayerInput
{
    GENERATED_BODY()

    /////////////////////////////////////////////////////////////////////////////////////////////////

protected:
    virtual void PostInitProperties() override;

    /**
    * NOTE:
    * With UInputSettings, there was a problem with user settings conflicting with project defaults.
    * We want developers to be able to use their own settings for PIE.
    * When changing keybindings in PIE, they are saved in Saved/Config/Input.ini.
    * However, editor loads project settings from both DefaultInput.ini and Input.ini afterwards.
    * Therefore, project settings are altered by developer personal config.
    * If developer adds or modifies a mapping in project settings, his entire config is then saved into DefaultConfig.ini.
    *
    * Solution:
    * - Keep UInputSettings vanilla for default project settings.
    * - Store user config in separate config section (here in UR_PlayerInput).
    * - Override mappings on load, except in editor mode.
    */

    //NOTE: Later on, we will probably want these settings externalized (cloud...)

    UPROPERTY(Config)
    TArray<FInputActionKeyMapping> UserActionMappings;

    UPROPERTY(Config)
    TArray<FInputAxisKeyMapping> UserAxisMappings;

    UPROPERTY(Config)
    TArray<FInputAxisConfigEntry> UserAxisConfigs;

    /**
    * Inject new project defaults into user settings (if any) and activate user settings.
    */
    UFUNCTION()
    virtual void SetupUserSettings();

    UFUNCTION()
    virtual void SaveUserSettings();

    /**
    * Activates current user settings.
    * Automatically injects our Tap* actions derived from movement axis.
    * Calls ForceRebuildingKeyMaps().
    */
    UFUNCTION()
    virtual void RegenerateInternalBindings();

    /////////////////////////////////////////////////////////////////////////////////////////////////

public:
    /**
    * Modify a key mapping for an action or axis
    * returns true if the key mapping was modified successfully, otherwise returns false
    */
    bool ModifyKeyMapping(const FName& MappingName, const FInputChord& InputChord);

protected:
    /**
    * Remap the given action to the given key
    */
    virtual void RemapAction(FInputActionKeyMapping& ActionKeyMapping, const FKey& Key);

    /**
    * Remap the given axis to the given key
    */
    virtual void RemapAxis(FInputAxisKeyMapping& AxisKeyMapping, const FKey& Key);

    /////////////////////////////////////////////////////////////////////////////////////////////////

public:
    UFUNCTION(BlueprintPure)
    virtual bool FindUserActionMappings(FName ActionName, TArray<FInputActionKeyMapping>& OutMappings) const;

    UFUNCTION(BlueprintPure)
    virtual bool FindUserAxisMappings(FName AxisName, TArray<FInputAxisKeyMapping>& OutMappings) const;

    //NOTE: structures FInputAxisConfigEntry / FInputAxisProperties are not exposed to blueprints.
    UFUNCTION()
    virtual bool FindUserAxisConfig(FName AxisKeyName, FInputAxisConfigEntry& OutAxisConfig) const;

    UFUNCTION(BlueprintPure)
    virtual void K2_GetUserAxisConfig(FName AxisKeyName, bool& bFound, float& Sensitivity, bool& bInvert);

    UFUNCTION(BlueprintCallable)
    virtual bool K2_UpdateUserAxisConfig(FName AxisKeyName, float Sensitivity = 1.f, bool bInvert = false);

    UFUNCTION(BlueprintPure)
    virtual bool AxisShouldGenerateTapAction(FName AxisName, FName& OutTapActionName);
};
