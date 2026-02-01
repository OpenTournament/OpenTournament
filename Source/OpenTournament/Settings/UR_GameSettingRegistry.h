// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "DataSource/GameSettingDataSourceDynamic.h" // IWYU pragma: keep
#include "GameSettingRegistry.h"
#include "Settings/UR_SettingsLocal.h" // IWYU pragma: keep

#include "UR_GameSettingRegistry.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class ULocalPlayer;
class UObject;

/////////////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------
// UUR_GameSettingRegistry
//--------------------------------------

class UGameSettingCollection;
class UUR_LocalPlayer;

/////////////////////////////////////////////////////////////////////////////////////////////////

DECLARE_LOG_CATEGORY_EXTERN(LogGameSettingRegistry, Log, Log);

#define GET_SHARED_SETTINGS_FUNCTION_PATH(FunctionOrPropertyName)							\
	MakeShared<FGameSettingDataSourceDynamic>(TArray<FString>({								\
		GET_FUNCTION_NAME_STRING_CHECKED(UUR_LocalPlayer, GetSharedSettings),				\
		GET_FUNCTION_NAME_STRING_CHECKED(UUR_SettingsShared, FunctionOrPropertyName)		\
	}))

#define GET_LOCAL_SETTINGS_FUNCTION_PATH(FunctionOrPropertyName)							\
	MakeShared<FGameSettingDataSourceDynamic>(TArray<FString>({								\
		GET_FUNCTION_NAME_STRING_CHECKED(UUR_LocalPlayer, GetLocalSettings),				\
		GET_FUNCTION_NAME_STRING_CHECKED(UUR_SettingsLocal, FunctionOrPropertyName)		\
	}))

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
UCLASS()
class UUR_GameSettingRegistry : public UGameSettingRegistry
{
	GENERATED_BODY()

public:
	UUR_GameSettingRegistry();

	static UUR_GameSettingRegistry* Get(UUR_LocalPlayer* InLocalPlayer);

	virtual void SaveChanges() override;

protected:
	virtual void OnInitialize(ULocalPlayer* InLocalPlayer) override;
	virtual bool IsFinishedInitializing() const override;

	UGameSettingCollection* InitializeVideoSettings(UUR_LocalPlayer* InLocalPlayer);
	void InitializeVideoSettings_FrameRates(UGameSettingCollection* Screen, UUR_LocalPlayer* InLocalPlayer);
	void AddPerformanceStatPage(UGameSettingCollection* Screen, UUR_LocalPlayer* InLocalPlayer);

	UGameSettingCollection* InitializeAudioSettings(UUR_LocalPlayer* InLocalPlayer);
	UGameSettingCollection* InitializeGameplaySettings(UUR_LocalPlayer* InLocalPlayer);

	UGameSettingCollection* InitializeMouseAndKeyboardSettings(UUR_LocalPlayer* InLocalPlayer);
	UGameSettingCollection* InitializeGamepadSettings(UUR_LocalPlayer* InLocalPlayer);

	UPROPERTY()
	TObjectPtr<UGameSettingCollection> VideoSettings;

	UPROPERTY()
	TObjectPtr<UGameSettingCollection> AudioSettings;

	UPROPERTY()
	TObjectPtr<UGameSettingCollection> GameplaySettings;

	UPROPERTY()
	TObjectPtr<UGameSettingCollection> MouseAndKeyboardSettings;

	UPROPERTY()
	TObjectPtr<UGameSettingCollection> GamepadSettings;
};
