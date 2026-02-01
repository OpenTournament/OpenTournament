// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_GameSettingRegistry.h"

#include "GameSettingCollection.h"
#include "UR_SettingsLocal.h"
#include "UR_SettingsShared.h"
#include "Player/UR_LocalPlayer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_GameSettingRegistry)

/////////////////////////////////////////////////////////////////////////////////////////////////

DEFINE_LOG_CATEGORY(LogGameSettingRegistry);

/////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "OT"

//--------------------------------------
// UUR_GameSettingRegistry
//--------------------------------------

UUR_GameSettingRegistry::UUR_GameSettingRegistry()
{
}

UUR_GameSettingRegistry* UUR_GameSettingRegistry::Get(UUR_LocalPlayer* InLocalPlayer)
{
	UUR_GameSettingRegistry* Registry = FindObject<UUR_GameSettingRegistry>(InLocalPlayer, TEXT("UR_GameSettingRegistry"), EFindObjectFlags::ExactClass);
	if (Registry == nullptr)
	{
		Registry = NewObject<UUR_GameSettingRegistry>(InLocalPlayer, TEXT("UR_GameSettingRegistry"));
		Registry->Initialize(InLocalPlayer);
	}

	return Registry;
}

bool UUR_GameSettingRegistry::IsFinishedInitializing() const
{
	if (Super::IsFinishedInitializing())
	{
		if (UUR_LocalPlayer* LocalPlayer = Cast<UUR_LocalPlayer>(OwningLocalPlayer))
		{
			if (LocalPlayer->GetSharedSettings() == nullptr)
			{
				return false;
			}
		}

		return true;
	}

	return false;
}

void UUR_GameSettingRegistry::OnInitialize(ULocalPlayer* InLocalPlayer)
{
	UUR_LocalPlayer* GameLocalPlayer = Cast<UUR_LocalPlayer>(InLocalPlayer);

	VideoSettings = InitializeVideoSettings(GameLocalPlayer);
	InitializeVideoSettings_FrameRates(VideoSettings, GameLocalPlayer);
	RegisterSetting(VideoSettings);

	AudioSettings = InitializeAudioSettings(GameLocalPlayer);
	RegisterSetting(AudioSettings);

	GameplaySettings = InitializeGameplaySettings(GameLocalPlayer);
	RegisterSetting(GameplaySettings);

	MouseAndKeyboardSettings = InitializeMouseAndKeyboardSettings(GameLocalPlayer);
	RegisterSetting(MouseAndKeyboardSettings);

	GamepadSettings = InitializeGamepadSettings(GameLocalPlayer);
	RegisterSetting(GamepadSettings);
}

void UUR_GameSettingRegistry::SaveChanges()
{
	Super::SaveChanges();

	if (UUR_LocalPlayer* LocalPlayer = Cast<UUR_LocalPlayer>(OwningLocalPlayer))
	{
		// Game user settings need to be applied to handle things like resolution, this saves indirectly
		LocalPlayer->GetLocalSettings()->ApplySettings(false);

		LocalPlayer->GetSharedSettings()->ApplySettings();
		LocalPlayer->GetSharedSettings()->SaveSettings();
	}
}

#undef LOCTEXT_NAMESPACE

