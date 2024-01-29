// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameSettingValue.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameSettingValue)

#define LOCTEXT_NAMESPACE "GameSetting"

//--------------------------------------
// UGameSettingValue
//--------------------------------------

UGameSettingValue::UGameSettingValue()
{
	// Values will report to analytics.
	bReportAnalytics = true;
}

void UGameSettingValue::OnInitialized()
{
	Super::OnInitialized();

#if !UE_BUILD_SHIPPING
	ensureAlwaysMsgf(!DescriptionRichText.IsEmpty() || DynamicDetails.IsBound(), TEXT("You must provide a description or it must specify a dynamic details function for settings with values."));
#endif

	StoreInitial();
}

#undef LOCTEXT_NAMESPACE

