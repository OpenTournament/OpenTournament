// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUserWidget.h"
#include "Containers/Ticker.h"
#include "GameSettingFilterState.h"
#include "GameplayTagContainer.h"

#include "GameSettingPanel.generated.h"

struct FCompiledToken;

class UGameSetting;
class UGameSettingDetailView;
class UGameSettingListView;
class UGameSettingRegistry;
class UObject;
struct FFocusEvent;
struct FGeometry;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnFocusedSettingChanged, UGameSetting*)

UCLASS(Abstract)
class GAMESETTINGS_API UGameSettingPanel : public UCommonUserWidget
{
	GENERATED_BODY()

public:

	UGameSettingPanel();
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// Focus transitioning to subwidgets for the gamepad
	virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;

	/**  */
	void SetRegistry(UGameSettingRegistry* InRegistry);

	/** Sets the filter for this panel, restricting which settings are available currently. */
	void SetFilterState(const FGameSettingFilterState& InFilterState, bool bClearNavigationStack = true);

	/** Gets the currently visible and available settings based on the filter state. */
	TArray<UGameSetting*> GetVisibleSettings() const { return VisibleSettings; }

	/** Can we pop the current navigation stack */
	bool CanPopNavigationStack() const;

	/** Pop the navigation stack */
	void PopNavigationStack();

	/**
	 * Gets the set of settings that are potentially available on this screen.
	 * MAY CONTAIN INVISIBLE SETTINGS.
	 * DOES NOT INCLUDED NESTED PAGES.
	 */
	TArray<UGameSetting*> GetSettingsWeCanResetToDefault() const;

	void SelectSetting(const FName& SettingDevName);
	UGameSetting* GetSelectedSetting() const;

	void RefreshSettingsList();

	FOnFocusedSettingChanged OnFocusedSettingChanged;

protected:
	void RegisterRegistryEvents();
	void UnregisterRegistryEvents();
	
	void HandleSettingItemHoveredChanged(UObject* Item, bool bHovered);
	void HandleSettingItemSelectionChanged(UObject* Item);
	void FillSettingDetails(UGameSetting* InSetting);
	void HandleSettingNamedAction(UGameSetting* Setting, FGameplayTag GameSettings_Action_Tag);
	void HandleSettingNavigation(UGameSetting* Setting);
	void HandleSettingEditConditionsChanged(UGameSetting* Setting);

private:

	UPROPERTY(Transient)
	TObjectPtr<UGameSettingRegistry> Registry;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UGameSetting>> VisibleSettings;

	UPROPERTY(Transient)
	TObjectPtr<UGameSetting> LastHoveredOrSelectedSetting;

	UPROPERTY(Transient)
	FGameSettingFilterState FilterState;

	UPROPERTY(Transient)
	TArray<FGameSettingFilterState> FilterNavigationStack;

	FName DesiredSelectionPostRefresh;

	bool bAdjustListViewPostRefresh = true;

private:	// Bound Widgets
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UGameSettingListView> ListView_Settings;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UGameSettingDetailView> Details_Settings;

private:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExecuteNamedActionBP, UGameSetting*, Setting, FGameplayTag, Action);
	UPROPERTY(BlueprintAssignable, Category = Events, meta = (DisplayName = "On Execute Named Action"))
	FOnExecuteNamedActionBP BP_OnExecuteNamedAction;

private:
	FTSTicker::FDelegateHandle RefreshHandle;
};
