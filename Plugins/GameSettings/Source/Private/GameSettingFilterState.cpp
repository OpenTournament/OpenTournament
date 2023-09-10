// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameSettingFilterState.h"
#include "GameSetting.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameSettingFilterState)

#define LOCTEXT_NAMESPACE "GameSetting"

class FSettingFilterExpressionContext : public ITextFilterExpressionContext
{
public:
	explicit FSettingFilterExpressionContext(const UGameSetting& InSetting) : Setting(InSetting) {}

	virtual bool TestBasicStringExpression(const FTextFilterString& InValue, const ETextFilterTextComparisonMode InTextComparisonMode) const override
	{
		return TextFilterUtils::TestBasicStringExpression(Setting.GetDescriptionPlainText(), InValue, InTextComparisonMode);
	}

	virtual bool TestComplexExpression(const FName& InKey, const FTextFilterString& InValue, const ETextFilterComparisonOperation InComparisonOperation, const ETextFilterTextComparisonMode InTextComparisonMode) const override
	{
		return false;
	}

private:
	/** Setting being filtered. */
	const UGameSetting& Setting;
};

//--------------------------------------
// FGameSettingFilterState
//--------------------------------------

FGameSettingFilterState::FGameSettingFilterState()
	: SearchTextEvaluator(ETextFilterExpressionEvaluatorMode::BasicString)
{
}

void FGameSettingFilterState::AddSettingToRootList(UGameSetting* InSetting)
{
	SettingAllowList.Add(InSetting);
	SettingRootList.Add(InSetting);
}

void FGameSettingFilterState::AddSettingToAllowList(UGameSetting* InSetting)
{
	SettingAllowList.Add(InSetting);
}

void FGameSettingFilterState::SetSearchText(const FString& InSearchText)
{
	SearchTextEvaluator.SetFilterText(FText::FromString(InSearchText));
}

bool FGameSettingFilterState::DoesSettingPassFilter(const UGameSetting& InSetting) const
{
	const FGameSettingEditableState& EditableState = InSetting.GetEditState();

	if (!bIncludeHidden && !EditableState.IsVisible())
	{
		return false;
	}

	if (!bIncludeDisabled && !EditableState.IsEnabled())
	{
		return false;
	}

	if (!bIncludeResetable && !EditableState.IsResetable())
	{
		return false;
	}

	// Are we filtering settings?
	if (SettingAllowList.Num() > 0)
	{
		if (!SettingAllowList.Contains(&InSetting))
		{
			bool bAllowed = false;
			const UGameSetting* NextSetting = &InSetting;
			while (const UGameSetting* Parent = NextSetting->GetSettingParent())
			{
				if (SettingAllowList.Contains(Parent))
				{
					bAllowed = true;
					break;
				}

				NextSetting = Parent;
			}

			if (!bAllowed)
			{
				return false;
			}
		}
	}

	// TODO more filters...

	// Always search text last, it's generally the most expensive filter.
	if (!SearchTextEvaluator.TestTextFilter(FSettingFilterExpressionContext(InSetting)))
	{
		return false;
	}

	return true;
}

//--------------------------------------
// FGameSettingsEditableState
//--------------------------------------

void FGameSettingEditableState::Hide(const FString& DevReason)
{
#if !UE_BUILD_SHIPPING
	ensureAlwaysMsgf(!DevReason.IsEmpty(), TEXT("To hide a setting, you must provide a developer reason."));
#endif

	bVisible = false;

#if !UE_BUILD_SHIPPING
	HiddenReasons.Add(DevReason);
#endif
}

void FGameSettingEditableState::Disable(const FText& Reason)
{
#if !UE_BUILD_SHIPPING
	ensureAlwaysMsgf(!Reason.IsEmpty(), TEXT("To disable a setting, you must provide a reason that we can show players."));
#endif

	bEnabled = false;
	DisabledReasons.Add(Reason);
}

void FGameSettingEditableState::DisableOption(const FString& Option)
{
#if !UE_BUILD_SHIPPING
	ensureAlwaysMsgf(!DisabledOptions.Contains(Option), TEXT("You've already disabled this option."));
#endif

	DisabledOptions.Add(Option);
}

void FGameSettingEditableState::UnableToReset()
{
	bResetable = false;
}

#undef LOCTEXT_NAMESPACE

