// Copyright Epic Games, Inc. All Rights Reserved.

#include "Widgets/GameSettingScreen.h"

#include "GameSettingCollection.h"
#include "Widgets/GameSettingPanel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameSettingScreen)

class UWidget;

#define LOCTEXT_NAMESPACE "GameSetting"

void UGameSettingScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UGameSettingScreen::NativeOnActivated()
{
	Super::NativeOnActivated();
	
	ChangeTracker.WatchRegistry(Registry);

	OnSettingsDirtyStateChanged(HaveSettingsBeenChanged());
}

void UGameSettingScreen::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();
}

UGameSettingRegistry* UGameSettingScreen::GetOrCreateRegistry()
{
	if (Registry == nullptr)
	{
		UGameSettingRegistry* NewRegistry = this->CreateRegistry();
		NewRegistry->OnSettingChangedEvent.AddUObject(this, &ThisClass::HandleSettingChanged);

		Settings_Panel->SetRegistry(NewRegistry);

		Registry = NewRegistry;
	}

	return Registry;
}

UWidget* UGameSettingScreen::NativeGetDesiredFocusTarget() const
{
	if (UWidget* Target = BP_GetDesiredFocusTarget())
	{
		return Target;
	}

	return Settings_Panel;
}

void UGameSettingScreen::ApplyChanges()
{
	if (ChangeTracker.HaveSettingsBeenChanged())
	{
		ChangeTracker.ApplyChanges();
		ClearDirtyState();
		Registry->SaveChanges();
	}
}

void UGameSettingScreen::CancelChanges()
{
	ChangeTracker.RestoreToInitial();
	ClearDirtyState();
}

void UGameSettingScreen::ClearDirtyState()
{
	ChangeTracker.ClearDirtyState();

	OnSettingsDirtyStateChanged(false);
}

bool UGameSettingScreen::AttemptToPopNavigation()
{
	if (Settings_Panel->CanPopNavigationStack())
	{
		Settings_Panel->PopNavigationStack();
		return true;
	}

	return false;
}

UGameSettingCollection* UGameSettingScreen::GetSettingCollection(FName SettingDevName, bool& HasAnySettings)
{
	HasAnySettings = false;
	
	if (UGameSettingCollection* Collection = GetRegistry()->FindSettingByDevNameChecked<UGameSettingCollection>(SettingDevName))
	{
		TArray<UGameSetting*> InOutSettings;
		
		FGameSettingFilterState FilterState;
		Collection->GetSettingsForFilter(FilterState, InOutSettings);

		HasAnySettings = InOutSettings.Num() > 0;
		
		return Collection;
	}

	return nullptr;
}

void UGameSettingScreen::NavigateToSetting(FName SettingDevName)
{
	NavigateToSettings({SettingDevName});
}

void UGameSettingScreen::NavigateToSettings(const TArray<FName>& SettingDevNames)
{
	FGameSettingFilterState FilterState;

	for (const FName SettingDevName : SettingDevNames)
	{
		if (UGameSetting* Setting = GetRegistry()->FindSettingByDevNameChecked<UGameSetting>(SettingDevName))
		{
			FilterState.AddSettingToRootList(Setting);
		}
	}
	
	Settings_Panel->SetFilterState(FilterState);
}

void UGameSettingScreen::HandleSettingChanged(UGameSetting* Setting, EGameSettingChangeReason Reason)
{
	OnSettingsDirtyStateChanged(true);
}

#undef LOCTEXT_NAMESPACE
