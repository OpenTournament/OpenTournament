// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Misc/TextFilterExpressionEvaluator.h"

#include "UObject/ObjectPtr.h"
#include "GameSettingFilterState.generated.h"

#define UE_API GAMESETTINGS_API

class ULocalPlayer;
class UGameSetting;
class UGameSettingCollection;

/** Why did the setting change? */
enum class EGameSettingChangeReason : uint8
{
	Change,
	DependencyChanged,
	ResetToDefault,
	RestoreToInitial,
};

/**
 * The filter state is intended to be any and all filtering we support.
 */
USTRUCT()
struct FGameSettingFilterState
{
	GENERATED_BODY()

public:

	UE_API FGameSettingFilterState();

	UPROPERTY()
	bool bIncludeDisabled = true;

	UPROPERTY()
	bool bIncludeHidden = false;

	UPROPERTY()
	bool bIncludeResetable = true;

	UPROPERTY()
	bool bIncludeNestedPages = false;

public:
	UE_API void SetSearchText(const FString& InSearchText);

	UE_API bool DoesSettingPassFilter(const UGameSetting& InSetting) const;

	UE_API void AddSettingToRootList(UGameSetting* InSetting);
	UE_API void AddSettingToAllowList(UGameSetting* InSetting);

	bool IsSettingInAllowList(const UGameSetting* InSetting) const
	{
		return SettingAllowList.Contains(InSetting);
	}
	
	const TArray<UGameSetting*>& GetSettingRootList() const { return SettingRootList; }
	bool IsSettingInRootList(const UGameSetting* InSetting) const
	{
		return SettingRootList.Contains(InSetting);
	}

private:
	FTextFilterExpressionEvaluator SearchTextEvaluator;

	UPROPERTY()
	TArray<TObjectPtr<UGameSetting>> SettingRootList;

	// If this is non-empty, then only settings in here are allowed
	UPROPERTY()
	TArray<TObjectPtr<UGameSetting>> SettingAllowList;
};

/**
 * Editable state captures the current visibility and enabled state of a setting. As well
 * as the reasons it got into that state.
 */
class FGameSettingEditableState
{
public:
	FGameSettingEditableState()
		: bVisible(true)
		, bEnabled(true)
		, bResetable(true)
		, bHideFromAnalytics(false)
	{
	}

	bool IsVisible() const { return bVisible; }
	bool IsEnabled() const { return bEnabled; }
	bool IsResetable() const { return bResetable; }
	bool IsHiddenFromAnalytics() const { return bHideFromAnalytics; }
	const TArray<FText>& GetDisabledReasons() const { return DisabledReasons; }

#if !UE_BUILD_SHIPPING
	const TArray<FString>& GetHiddenReasons() const { return HiddenReasons; }
#endif

	const TArray<FString>& GetDisabledOptions() const { return DisabledOptions; }

	/** Hides the setting, you don't have to provide a user facing reason, but you do need to specify a developer reason. */
	UE_API void Hide(const FString& DevReason);

	/** Disables the setting, you need to provide a reason you disabled this setting. */
	UE_API void Disable(const FText& Reason);

	/** Discrete Options that should be hidden from the user. Currently used only by Parental Controls. */
	UE_API void DisableOption(const FString& Option);

	template<typename EnumType>
	void DisableEnumOption(EnumType InEnumValue)
	{
		DisableOption(StaticEnum<EnumType>()->GetNameStringByValue((int64)InEnumValue));
	}

	/**
	 * Prevents the setting from being reset if the user resets the settings on the screen to their defaults.
	 */
	UE_API void UnableToReset();

	/**
	 * Hide from analytics, you may want to do this if for example, we just want to prevent noise, such as platform
	 * specific edit conditions where it doesn't make sense to report settings for platforms where they don't exist.
	 */
	void HideFromAnalytics() { bHideFromAnalytics = true; }

	/** Hides it in every way possible.  Hides it visually.  Marks it as Immutable for being reset.  Hides it from analytics. */
	void Kill(const FString& DevReason)
	{
		Hide(DevReason);
		HideFromAnalytics();
		UnableToReset();
	}

private:
	uint8 bVisible : 1;
	uint8 bEnabled : 1;
	uint8 bResetable : 1;
	uint8 bHideFromAnalytics : 1;

	TArray<FString> DisabledOptions;

	TArray<FText> DisabledReasons;

#if !UE_BUILD_SHIPPING
	TArray<FString> HiddenReasons;
#endif
};

/**
 * Edit conditions can monitor the state of the game or of other settings and adjust the 
 * visibility.
 */
class FGameSettingEditCondition : public TSharedFromThis<FGameSettingEditCondition>
{
public:
	FGameSettingEditCondition() { }
	virtual ~FGameSettingEditCondition() { }

	DECLARE_EVENT_OneParam(FGameSettingEditCondition, FOnEditConditionChanged, bool);
	FOnEditConditionChanged OnEditConditionChangedEvent;

	/** Broadcasts Event*/
	void BroadcastEditConditionChanged()
	{
		OnEditConditionChangedEvent.Broadcast(true);
	}

	/** Called during the setting Initialization */
	virtual void Initialize(const ULocalPlayer* InLocalPlayer)
	{
	}

	/** Called when the setting is 'applied'. */
	virtual void SettingApplied(const ULocalPlayer* InLocalPlayer, UGameSetting* Setting) const
	{
	}

	/** Called when the setting is changed. */
	virtual void SettingChanged(const ULocalPlayer* InLocalPlayer, UGameSetting* Setting, EGameSettingChangeReason Reason) const
	{
	}

	/**
	 * Called when the setting needs to re-evaluate edit state. Usually this is in response to a 
	 * dependency changing, or if this edit condition emits an OnEditConditionChangedEvent.
	 */
	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer, FGameSettingEditableState& InOutEditState) const
	{
	}

	/** Generate useful debugging text for this edit condition.  Helpful when things don't work as expected. */
	virtual FString ToString() const { return TEXT(""); }
};

#undef UE_API
