// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameSetting.h"

#include "GameSettingValue.generated.h"

class UObject;

//--------------------------------------
// UGameSettingValue
//--------------------------------------

/**
 * The base class for all settings that are conceptually a value, that can be 
 * changed, and thus reset or restored to their initial value.
 */
UCLASS(Abstract)
class GAMESETTINGS_API UGameSettingValue : public UGameSetting
{
	GENERATED_BODY()

public:
	UGameSettingValue();

	/** Stores an initial value for the setting.  This will be called on initialize, but should also be called if you 'apply' the setting. */
	virtual void StoreInitial() PURE_VIRTUAL(, );

	/** Resets the property to the default. */
	virtual void ResetToDefault() PURE_VIRTUAL(, );

	/** Restores the setting to the initial value, this is the value when you open the settings before making any tweaks. */
	virtual void RestoreToInitial() PURE_VIRTUAL(, );

protected:
	virtual void OnInitialized() override;
};
