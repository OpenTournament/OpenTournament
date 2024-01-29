// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameSetting.h"
#include "Templates/Casts.h"

#include "GameSettingRegistry.generated.h"

struct FGameplayTag;

//--------------------------------------
// UGameSettingRegistry
//--------------------------------------

class ULocalPlayer;
struct FGameSettingFilterState;

enum class EGameSettingChangeReason : uint8;

/**
 * 
 */
UCLASS(Abstract, BlueprintType)
class GAMESETTINGS_API UGameSettingRegistry : public UObject
{
	GENERATED_BODY()

public:
	DECLARE_EVENT_TwoParams(UGameSettingRegistry, FOnSettingChanged, UGameSetting*, EGameSettingChangeReason);
	DECLARE_EVENT_OneParam(UGameSettingRegistry, FOnSettingEditConditionChanged, UGameSetting*);

	FOnSettingChanged OnSettingChangedEvent;
	FOnSettingEditConditionChanged OnSettingEditConditionChangedEvent;

	DECLARE_EVENT_TwoParams(UGameSettingRegistry, FOnSettingNamedActionEvent, UGameSetting* /*Setting*/, FGameplayTag /*GameSettings_Action_Tag*/);
	FOnSettingNamedActionEvent OnSettingNamedActionEvent;

	/** Navigate to the child settings of the provided setting. */
	DECLARE_EVENT_OneParam(UGameSettingRegistry, FOnExecuteNavigation, UGameSetting* /*Setting*/);
	FOnExecuteNavigation OnExecuteNavigationEvent;

public:
	UGameSettingRegistry();

	void Initialize(ULocalPlayer* InLocalPlayer);

	virtual void Regenerate();

	virtual bool IsFinishedInitializing() const;

	virtual void SaveChanges();
	
	void GetSettingsForFilter(const FGameSettingFilterState& FilterState, TArray<UGameSetting*>& InOutSettings);

	UGameSetting* FindSettingByDevName(const FName& SettingDevName);

	template<typename T = UGameSetting>
	T* FindSettingByDevNameChecked(const FName& SettingDevName)
	{
		T* Setting = Cast<T>(FindSettingByDevName(SettingDevName));
		check(Setting);
		return Setting;
	}

protected:
	virtual void OnInitialize(ULocalPlayer* InLocalPlayer) PURE_VIRTUAL(, )

	virtual void OnSettingApplied(UGameSetting* Setting) { }
	
	void RegisterSetting(UGameSetting* InSetting);
	void RegisterInnerSettings(UGameSetting* InSetting);

	// Internal event handlers.
	void HandleSettingChanged(UGameSetting* Setting, EGameSettingChangeReason Reason);
	void HandleSettingApplied(UGameSetting* Setting);
	void HandleSettingEditConditionsChanged(UGameSetting* Setting);
	void HandleSettingNamedAction(UGameSetting* Setting, FGameplayTag GameSettings_Action_Tag);
	void HandleSettingNavigation(UGameSetting* Setting);

	UPROPERTY(Transient)
	TArray<TObjectPtr<UGameSetting>> TopLevelSettings;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UGameSetting>> RegisteredSettings;

	UPROPERTY(Transient)
	TObjectPtr<ULocalPlayer> OwningLocalPlayer;
};
