// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_SettingValueDiscrete_MobileFPSType.h"

#include "Performance/UR_PerformanceSettings.h"
#include "Settings/UR_SettingsLocal.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_SettingValueDiscrete_MobileFPSType)

/////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "OTSettings"

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_SettingValueDiscrete_MobileFPSType::UUR_SettingValueDiscrete_MobileFPSType()
{}

void UUR_SettingValueDiscrete_MobileFPSType::OnInitialized()
{
    Super::OnInitialized();

    const UUR_PlatformSpecificRenderingSettings* PlatformSettings = UUR_PlatformSpecificRenderingSettings::Get();
    const UUR_SettingsLocal* UserSettings = UUR_SettingsLocal::Get();

    for (int32 TestLimit : PlatformSettings->MobileFrameRateLimits)
    {
        if (UUR_SettingsLocal::IsSupportedMobileFramePace(TestLimit))
        {
            FPSOptions.Add(TestLimit, MakeLimitString(TestLimit));
        }
    }

    const int32 FirstFrameRateWithQualityLimit = UserSettings->GetFirstFrameRateWithQualityLimit();
    if (FirstFrameRateWithQualityLimit > 0)
    {
        SetWarningRichText(FText::Format(LOCTEXT("MobileFPSType_Note", "<strong>Note: Changing the framerate setting to {0} or higher might lower your Quality Presets.</>"), MakeLimitString(FirstFrameRateWithQualityLimit)));
    }
}

int32 UUR_SettingValueDiscrete_MobileFPSType::GetDefaultFPS() const
{
    return UUR_SettingsLocal::GetDefaultMobileFrameRate();
}

FText UUR_SettingValueDiscrete_MobileFPSType::MakeLimitString(int32 Number)
{
    return FText::Format(LOCTEXT("MobileFrameRateOption", "{0} FPS"), FText::AsNumber(Number));
}

void UUR_SettingValueDiscrete_MobileFPSType::StoreInitial()
{
    InitialValue = GetValue();
}

void UUR_SettingValueDiscrete_MobileFPSType::ResetToDefault()
{
    SetValue(GetDefaultFPS(), EGameSettingChangeReason::ResetToDefault);
}

void UUR_SettingValueDiscrete_MobileFPSType::RestoreToInitial()
{
    SetValue(InitialValue, EGameSettingChangeReason::RestoreToInitial);
}

void UUR_SettingValueDiscrete_MobileFPSType::SetDiscreteOptionByIndex(int32 Index)
{
    TArray<int32> FPSOptionsModes;
    FPSOptions.GenerateKeyArray(FPSOptionsModes);

    int32 NewMode = FPSOptionsModes.IsValidIndex(Index) ? FPSOptionsModes[Index] : GetDefaultFPS();

    SetValue(NewMode, EGameSettingChangeReason::Change);
}

int32 UUR_SettingValueDiscrete_MobileFPSType::GetDiscreteOptionIndex() const
{
    TArray<int32> FPSOptionsModes;
    FPSOptions.GenerateKeyArray(FPSOptionsModes);
    return FPSOptionsModes.IndexOfByKey(GetValue());
}

TArray<FText> UUR_SettingValueDiscrete_MobileFPSType::GetDiscreteOptions() const
{
    TArray<FText> Options;
    FPSOptions.GenerateValueArray(Options);

    return Options;
}

int32 UUR_SettingValueDiscrete_MobileFPSType::GetValue() const
{
    return UUR_SettingsLocal::Get()->GetDesiredMobileFrameRateLimit();
}

void UUR_SettingValueDiscrete_MobileFPSType::SetValue(int32 NewLimitFPS, EGameSettingChangeReason InReason)
{
    UUR_SettingsLocal::Get()->SetDesiredMobileFrameRateLimit(NewLimitFPS);

    NotifySettingChanged(InReason);
}

#undef LOCTEXT_NAMESPACE
