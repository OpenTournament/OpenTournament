// Copyright Epic Games, Inc. All Rights Reserved.

#include "Widgets/GameSettingListEntry.h"

#include "AnalogSlider.h"
#include "CommonInputSubsystem.h"
#include "CommonInputTypeEnum.h"
#include "CommonTextBlock.h"
#include "GameSettingAction.h"
#include "GameSettingCollection.h"
#include "GameSettingValueDiscrete.h"
#include "GameSettingValueScalar.h"
#include "Widgets/Misc/GameSettingRotator.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameSettingListEntry)

class SWidget;
struct FGeometry;

#define LOCTEXT_NAMESPACE "GameSetting"

//////////////////////////////////////////////////////////////////////////
// UGameSettingListEntryBase
//////////////////////////////////////////////////////////////////////////

void UGameSettingListEntryBase::SetSetting(UGameSetting* InSetting)
{
	Setting = InSetting;
	Setting->OnSettingEditConditionChangedEvent.AddUObject(this, &ThisClass::HandleEditConditionChanged);
	Setting->OnSettingChangedEvent.AddUObject(this, &ThisClass::HandleSettingChanged);

	HandleEditConditionChanged(Setting);
}

void UGameSettingListEntryBase::SetDisplayNameOverride(const FText& OverrideName)
{
	DisplayNameOverride = OverrideName;
}

void UGameSettingListEntryBase::NativeOnEntryReleased()
{
	StopAllAnimations();

	if (Background)
	{
		Background->StopAllAnimations();
	}

	if (ensure(Setting))
	{
		Setting->OnSettingEditConditionChangedEvent.RemoveAll(this);
		Setting->OnSettingChangedEvent.RemoveAll(this);
	}

	Setting = nullptr;
}

void UGameSettingListEntryBase::HandleSettingChanged(UGameSetting* InSetting, EGameSettingChangeReason Reason)
{
	if (!bSuspendChangeUpdates)
	{
		OnSettingChanged();
	}
}

void UGameSettingListEntryBase::OnSettingChanged()
{
	// No-Op
}

void UGameSettingListEntryBase::HandleEditConditionChanged(UGameSetting* InSetting)
{
	const FGameSettingEditableState EditableState = Setting->GetEditState();
	RefreshEditableState(EditableState);
}

void UGameSettingListEntryBase::RefreshEditableState(const FGameSettingEditableState& InEditableState)
{
	// No-Op
}

FReply UGameSettingListEntryBase::NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent)
{
	const UCommonInputSubsystem* InputSubsystem = GetInputSubsystem();
	if (InputSubsystem && InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad)
	{
		if (UWidget* PrimaryFocus = GetPrimaryGamepadFocusWidget())
		{
			TSharedPtr<SWidget> WidgetToFocus = PrimaryFocus->GetCachedWidget();
			if (WidgetToFocus.IsValid())
			{
				return FReply::Handled().SetUserFocus(WidgetToFocus.ToSharedRef(), InFocusEvent.GetCause());
			}
		}
	}

	return FReply::Unhandled();
}

//////////////////////////////////////////////////////////////////////////
// UGameSettingListEntry_Setting
//////////////////////////////////////////////////////////////////////////

void UGameSettingListEntry_Setting::SetSetting(UGameSetting* InSetting)
{
	Super::SetSetting(InSetting);

	Text_SettingName->SetText(DisplayNameOverride.IsEmpty() ? Setting->GetDisplayName() : DisplayNameOverride);
	Text_SettingName->SetVisibility(InSetting->GetDisplayNameVisibility());
}

//////////////////////////////////////////////////////////////////////////
// UGameSettingListEntrySetting_Discrete
//////////////////////////////////////////////////////////////////////////

void UGameSettingListEntrySetting_Discrete::SetSetting(UGameSetting* InSetting)
{
	DiscreteSetting = Cast<UGameSettingValueDiscrete>(InSetting);

	Super::SetSetting(InSetting);
	
	Refresh();
}

void UGameSettingListEntrySetting_Discrete::Refresh()
{
	if (ensure(DiscreteSetting))
	{
		const TArray<FText> Options = DiscreteSetting->GetDiscreteOptions();
		ensure(Options.Num() > 0);

		Rotator_SettingValue->PopulateTextLabels(Options);
		Rotator_SettingValue->SetSelectedItem(DiscreteSetting->GetDiscreteOptionIndex());
		Rotator_SettingValue->SetDefaultOption(DiscreteSetting->GetDiscreteOptionDefaultIndex());
	}
}

void UGameSettingListEntrySetting_Discrete::RefreshEditableState(const FGameSettingEditableState& InEditableState)
{
	Super::RefreshEditableState(InEditableState);

	const bool bLocalIsEnabled = InEditableState.IsEnabled();
	Button_Decrease->SetIsEnabled(bLocalIsEnabled);
	Rotator_SettingValue->SetIsEnabled(bLocalIsEnabled);
	Button_Increase->SetIsEnabled(bLocalIsEnabled);
	Panel_Value->SetIsEnabled(bLocalIsEnabled);
}

void UGameSettingListEntrySetting_Discrete::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Rotator_SettingValue->OnRotatedEvent.AddUObject(this, &ThisClass::HandleRotatorChangedValue);
	Button_Decrease->OnClicked().AddUObject(this, &ThisClass::HandleOptionDecrease);
	Button_Increase->OnClicked().AddUObject(this, &ThisClass::HandleOptionIncrease);
}

void UGameSettingListEntrySetting_Discrete::NativeOnEntryReleased()
{
	Super::NativeOnEntryReleased();

	DiscreteSetting = nullptr;
}

void UGameSettingListEntrySetting_Discrete::HandleOptionDecrease()
{
	//TODO NDarnell Doing this through the UI feels wrong, should use Setting directly.
	Rotator_SettingValue->ShiftTextLeft();
	DiscreteSetting->SetDiscreteOptionByIndex(Rotator_SettingValue->GetSelectedIndex());
}

void UGameSettingListEntrySetting_Discrete::HandleOptionIncrease()
{
	//TODO NDarnell Doing this through the UI feels wrong, should use Setting directly.
	Rotator_SettingValue->ShiftTextRight();
	DiscreteSetting->SetDiscreteOptionByIndex(Rotator_SettingValue->GetSelectedIndex());
}

void UGameSettingListEntrySetting_Discrete::HandleRotatorChangedValue(int32 Value, bool bUserInitiated)
{
	if (bUserInitiated)
	{
		DiscreteSetting->SetDiscreteOptionByIndex(Value);
	}
}

void UGameSettingListEntrySetting_Discrete::OnSettingChanged()
{
	Refresh();
}

void UGameSettingListEntrySetting_Discrete::HandleEditConditionChanged(UGameSetting* InSetting)
{
	Super::HandleEditConditionChanged(InSetting);

	Refresh();
}


//////////////////////////////////////////////////////////////////////////
// UGameSettingListEntrySetting_Scalar
//////////////////////////////////////////////////////////////////////////

void UGameSettingListEntrySetting_Scalar::SetSetting(UGameSetting* InSetting)
{
	ScalarSetting = Cast<UGameSettingValueScalar>(InSetting);

	Super::SetSetting(InSetting);

	Refresh();
}

void UGameSettingListEntrySetting_Scalar::Refresh()
{
	if (ensure(ScalarSetting))
	{
		const float Value = ScalarSetting->GetValueNormalized();

		Slider_SettingValue->SetValue(Value);
		Slider_SettingValue->SetStepSize(ScalarSetting->GetNormalizedStepSize());
		Text_SettingValue->SetText(ScalarSetting->GetFormattedText());

		TOptional<double> DefaultValue = ScalarSetting->GetDefaultValueNormalized();
		OnDefaultValueChanged(DefaultValue.IsSet() ? DefaultValue.GetValue() : -1.0);

		OnValueChanged(Value);
	}
}

void UGameSettingListEntrySetting_Scalar::OnSettingChanged()
{
	Refresh();
}

void UGameSettingListEntrySetting_Scalar::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Slider_SettingValue->OnValueChanged.AddDynamic(this, &ThisClass::HandleSliderValueChanged);

	Slider_SettingValue->OnMouseCaptureEnd.AddDynamic(this, &ThisClass::HandleSliderCaptureEnded);
	Slider_SettingValue->OnControllerCaptureEnd.AddDynamic(this, &ThisClass::HandleSliderCaptureEnded);
}

void UGameSettingListEntrySetting_Scalar::NativeOnEntryReleased()
{
	Super::NativeOnEntryReleased();

	ScalarSetting = nullptr;
}

void UGameSettingListEntrySetting_Scalar::HandleSliderValueChanged(float Value)
{
	TGuardValue<bool> Suspend(bSuspendChangeUpdates, true);

	if (ensure(ScalarSetting))
	{
		ScalarSetting->SetValueNormalized(Value);
		Value = ScalarSetting->GetValueNormalized();

		Slider_SettingValue->SetValue(Value);
		Text_SettingValue->SetText(ScalarSetting->GetFormattedText());

		OnValueChanged(Value);
	}
}

void UGameSettingListEntrySetting_Scalar::HandleSliderCaptureEnded()
{
	TGuardValue<bool> Suspend(bSuspendChangeUpdates, true);

	//commit?
}

void UGameSettingListEntrySetting_Scalar::RefreshEditableState(const FGameSettingEditableState& InEditableState)
{
	Super::RefreshEditableState(InEditableState);

	const bool bLocalIsEnabled = InEditableState.IsEnabled();
	Slider_SettingValue->SetIsEnabled(bLocalIsEnabled);
	Panel_Value->SetIsEnabled(bLocalIsEnabled);
}

//////////////////////////////////////////////////////////////////////////
// UGameSettingListEntrySetting_Action
//////////////////////////////////////////////////////////////////////////

void UGameSettingListEntrySetting_Action::SetSetting(UGameSetting* InSetting)
{
	Super::SetSetting(InSetting);

	ActionSetting = Cast<UGameSettingAction>(InSetting);
	if (ensure(ActionSetting))
	{
		OnSettingAssigned(ActionSetting->GetActionText());
	}
}

void UGameSettingListEntrySetting_Action::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Button_Action->OnClicked().AddUObject(this, &ThisClass::HandleActionButtonClicked);
}

void UGameSettingListEntrySetting_Action::NativeOnEntryReleased()
{
	Super::NativeOnEntryReleased();

	ActionSetting = nullptr;
}

void UGameSettingListEntrySetting_Action::RefreshEditableState(const FGameSettingEditableState& InEditableState)
{
	Super::RefreshEditableState(InEditableState);

	const bool bLocalIsEnabled = InEditableState.IsEnabled();
	Button_Action->SetIsEnabled(bLocalIsEnabled);
}

void UGameSettingListEntrySetting_Action::HandleActionButtonClicked()
{
	ActionSetting->ExecuteAction();
}

//////////////////////////////////////////////////////////////////////////
// UGameSettingListEntrySetting_Navigation
//////////////////////////////////////////////////////////////////////////

void UGameSettingListEntrySetting_Navigation::SetSetting(UGameSetting* InSetting)
{
	CollectionSetting = Cast<UGameSettingCollectionPage>(InSetting);

	Super::SetSetting(InSetting);

	if (ensure(CollectionSetting))
	{
		OnSettingAssigned(CollectionSetting->GetNavigationText());
	}
}

void UGameSettingListEntrySetting_Navigation::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Button_Navigate->OnClicked().AddUObject(this, &ThisClass::HandleNavigationButtonClicked);
}

void UGameSettingListEntrySetting_Navigation::NativeOnEntryReleased()
{
	Super::NativeOnEntryReleased();

	CollectionSetting = nullptr;
}

void UGameSettingListEntrySetting_Navigation::RefreshEditableState(const FGameSettingEditableState& InEditableState)
{
	Super::RefreshEditableState(InEditableState);

	const bool bLocalIsEnabled = InEditableState.IsEnabled();
	Button_Navigate->SetIsEnabled(bLocalIsEnabled);
}

void UGameSettingListEntrySetting_Navigation::HandleNavigationButtonClicked()
{
	CollectionSetting->ExecuteNavigation();
}

#undef LOCTEXT_NAMESPACE
