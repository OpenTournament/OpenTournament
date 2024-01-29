// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/IUserObjectListEntry.h"
#include "CommonUserWidget.h"

#include "GameSettingListEntry.generated.h"

class FGameSettingEditableState;
enum class EGameSettingChangeReason : uint8;

class UAnalogSlider;
class UCommonButtonBase;
class UCommonTextBlock;
class UGameSetting;
class UGameSettingAction;
class UGameSettingCollectionPage;
class UGameSettingRotator;
class UGameSettingValueDiscrete;
class UGameSettingValueScalar;
class UObject;
class UPanelWidget;
class UUserWidget;
class UWidget;
struct FFocusEvent;
struct FFrame;
struct FGeometry;

//////////////////////////////////////////////////////////////////////////
// UAthenaChallengeListEntry
//////////////////////////////////////////////////////////////////////////

UCLASS(Abstract, NotBlueprintable, meta = (Category = "Settings", DisableNativeTick))
class GAMESETTINGS_API UGameSettingListEntryBase : public UCommonUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	virtual void SetSetting(UGameSetting* InSetting);
	virtual void SetDisplayNameOverride(const FText& OverrideName);

protected:
	virtual void NativeOnEntryReleased() override;
	virtual void OnSettingChanged();
	virtual void HandleEditConditionChanged(UGameSetting* InSetting);
	virtual void RefreshEditableState(const FGameSettingEditableState& InEditableState);
	
protected:
	// Focus transitioning to subwidgets for the gamepad
	virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;

	UFUNCTION(BlueprintImplementableEvent)
	UWidget* GetPrimaryGamepadFocusWidget();

protected:
	bool bSuspendChangeUpdates = false;

	UPROPERTY()
	TObjectPtr<UGameSetting> Setting;

	FText DisplayNameOverride = FText::GetEmpty();

private:
	void HandleSettingChanged(UGameSetting* InSetting, EGameSettingChangeReason Reason);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UUserWidget> Background;
};


//////////////////////////////////////////////////////////////////////////
// UGameSettingListEntry_Setting
//////////////////////////////////////////////////////////////////////////

UCLASS(Abstract, Blueprintable, meta = (Category = "Settings", DisableNativeTick))
class GAMESETTINGS_API UGameSettingListEntry_Setting : public UGameSettingListEntryBase
{
	GENERATED_BODY()

public:
	virtual void SetSetting(UGameSetting* InSetting) override;
	
private:	// Bound Widgets
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UCommonTextBlock> Text_SettingName;
};


//////////////////////////////////////////////////////////////////////////
// UGameSettingListEntrySetting_Discrete
//////////////////////////////////////////////////////////////////////////

UCLASS(Abstract, Blueprintable, meta = (Category = "Settings", DisableNativeTick))
class GAMESETTINGS_API UGameSettingListEntrySetting_Discrete : public UGameSettingListEntry_Setting
{
	GENERATED_BODY()

public:
	virtual void SetSetting(UGameSetting* InSetting) override;
	
protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnEntryReleased() override;

	void HandleOptionDecrease();
	void HandleOptionIncrease();
	void HandleRotatorChangedValue(int32 Value, bool bUserInitiated);

	void Refresh();
	virtual void OnSettingChanged() override;
	virtual void HandleEditConditionChanged(UGameSetting* InSetting) override;
	virtual void RefreshEditableState(const FGameSettingEditableState& InEditableState) override;

protected:
	UPROPERTY()
	TObjectPtr<UGameSettingValueDiscrete> DiscreteSetting;

private:	// Bound Widgets
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UPanelWidget> Panel_Value;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UGameSettingRotator> Rotator_SettingValue;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UCommonButtonBase> Button_Decrease;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UCommonButtonBase> Button_Increase;
};

//////////////////////////////////////////////////////////////////////////
// UGameSettingListEntrySetting_Scalar
//////////////////////////////////////////////////////////////////////////

UCLASS(Abstract, Blueprintable, meta = (Category = "Settings", DisableNativeTick))
class GAMESETTINGS_API UGameSettingListEntrySetting_Scalar : public UGameSettingListEntry_Setting
{
	GENERATED_BODY()

public:
	virtual void SetSetting(UGameSetting* InSetting) override;

protected:
	void Refresh();
	virtual void NativeOnInitialized() override;
	virtual void NativeOnEntryReleased() override;
	virtual void OnSettingChanged() override;

	UFUNCTION()
	void HandleSliderValueChanged(float Value);
	UFUNCTION()
	void HandleSliderCaptureEnded();

	UFUNCTION(BlueprintImplementableEvent)
	void OnValueChanged(float Value);

	UFUNCTION(BlueprintImplementableEvent)
	void OnDefaultValueChanged(float DefaultValue);

	virtual void RefreshEditableState(const FGameSettingEditableState& InEditableState) override;

protected:
	UPROPERTY()
	TObjectPtr<UGameSettingValueScalar> ScalarSetting;

private:	// Bound Widgets
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UPanelWidget> Panel_Value;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UAnalogSlider> Slider_SettingValue;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UCommonTextBlock> Text_SettingValue;
};


//////////////////////////////////////////////////////////////////////////
// UGameSettingListEntrySetting_Action
//////////////////////////////////////////////////////////////////////////

UCLASS(Abstract, Blueprintable, meta = (Category = "Settings", DisableNativeTick))
class GAMESETTINGS_API UGameSettingListEntrySetting_Action : public UGameSettingListEntry_Setting
{
	GENERATED_BODY()

public:
	virtual void SetSetting(UGameSetting* InSetting) override;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnEntryReleased() override;
	virtual void RefreshEditableState(const FGameSettingEditableState& InEditableState) override;

	void HandleActionButtonClicked();

	UFUNCTION(BlueprintImplementableEvent)
	void OnSettingAssigned(const FText& ActionText);

protected:
	UPROPERTY()
	TObjectPtr<UGameSettingAction> ActionSetting;

private:	// Bound Widgets

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UCommonButtonBase> Button_Action;
};

//////////////////////////////////////////////////////////////////////////
// UGameSettingListEntrySetting_Navigation
//////////////////////////////////////////////////////////////////////////

UCLASS(Abstract, Blueprintable, meta = (Category = "Settings", DisableNativeTick))
class GAMESETTINGS_API UGameSettingListEntrySetting_Navigation : public UGameSettingListEntry_Setting
{
	GENERATED_BODY()

public:
	virtual void SetSetting(UGameSetting* InSetting) override;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnEntryReleased() override;
	virtual void RefreshEditableState(const FGameSettingEditableState& InEditableState) override;

	void HandleNavigationButtonClicked();

	UFUNCTION(BlueprintImplementableEvent)
	void OnSettingAssigned(const FText& ActionText);

protected:
	UPROPERTY()
	TObjectPtr<UGameSettingCollectionPage> CollectionSetting;

private:	// Bound Widgets

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UCommonButtonBase> Button_Navigate;
};
