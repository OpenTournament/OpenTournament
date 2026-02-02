// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_SettingsLocal.h"

#include "AudioModulationStatics.h"
#include "CommonInputSubsystem.h"
#include "CommonUISettings.h"
#include "ICommonUIModule.h"
#include "SoundControlBus.h"
#include "SoundControlBusMix.h"
#include "DeviceProfiles/DeviceProfile.h"
#include "DeviceProfiles/DeviceProfileManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "GenericPlatform/GenericPlatformFramePacer.h"
#include "HAL/PlatformFramePacer.h"
#include "Misc/App.h"
#include "Performance/LatencyMarkerModule.h"
#include "Widgets/Layout/SSafeZone.h"

#include "Development/UR_PlatformEmulationSettings.h"
#include "Audio/UR_AudioMixEffectsSubsystem.h"
#include "Audio/UR_AudioSettings.h"
#include "Performance/UR_PerformanceSettings.h"
#include "Performance/UR_PerformanceStatTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_SettingsLocal)

/////////////////////////////////////////////////////////////////////////////////////////////////

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Platform_Trait_BinauralSettingControlledByOS, "Platform.Trait.BinauralSettingControlledByOS");

namespace PerfStatTags
{
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Platform_Trait_SupportsLatencyStats, "Platform.Trait.SupportsLatencyStats");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Platform_Trait_SupportsLatencyMarkers, "Platform.Trait.SupportsLatencyMarkers");
}

/////////////////////////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR

static TAutoConsoleVariable<bool> CVarApplyFrameRateSettingsInPIE
(
    TEXT("OT.Settings.ApplyFrameRateSettingsInPIE"),
    false,
    TEXT("Should we apply frame rate settings in PIE?"),
    ECVF_Default
);

static TAutoConsoleVariable<bool> CVarApplyFrontEndPerformanceOptionsInPIE
(
    TEXT("OT.Settings.ApplyFrontEndPerformanceOptionsInPIE"),
    false,
    TEXT("Do we apply front-end specific performance options in PIE?"),
    ECVF_Default
);

static TAutoConsoleVariable<bool> CVarApplyDeviceProfilesInPIE
(
    TEXT("OT.Settings.ApplyDeviceProfilesInPIE"),
    false,
    TEXT("Should we apply experience/platform emulated device profiles in PIE?"),
    ECVF_Default
);

#endif

//////////////////////////////////////////////////////////////////////
// Console frame pacing

static TAutoConsoleVariable<int32> CVarDeviceProfileDrivenTargetFps
(
    TEXT("OT.DeviceProfile.Console.TargetFPS"),
    -1,
    TEXT("Target FPS when being driven by device profile"),
    ECVF_Default | ECVF_Preview
);

static TAutoConsoleVariable<int32> CVarDeviceProfileDrivenFrameSyncType
(
    TEXT("OT.DeviceProfile.Console.FrameSyncType"),
    -1,
    TEXT("Sync type when being driven by device profile. Corresponds to r.GTSyncType"),
    ECVF_Default | ECVF_Preview
);

//////////////////////////////////////////////////////////////////////
// Mobile frame pacing

static TAutoConsoleVariable<int32> CVarDeviceProfileDrivenMobileDefaultFrameRate
(
    TEXT("OT.DeviceProfile.Mobile.DefaultFrameRate"),
    30,
    TEXT("Default FPS when being driven by device profile"),
    ECVF_Default | ECVF_Preview
);

static TAutoConsoleVariable<int32> CVarDeviceProfileDrivenMobileMaxFrameRate
(
    TEXT("OT.DeviceProfile.Mobile.MaxFrameRate"),
    30,
    TEXT("Max FPS when being driven by device profile"),
    ECVF_Default | ECVF_Preview
);

//////////////////////////////////////////////////////////////////////

static TAutoConsoleVariable<FString> CVarMobileQualityLimits
(
    TEXT("OT.DeviceProfile.Mobile.OverallQualityLimits"),
    TEXT(""),
    TEXT("List of limits on resolution quality of the form \"FPS:MaxQuality,FPS2:MaxQuality2,...\", kicking in when FPS is at or above the threshold"),
    ECVF_Default | ECVF_Preview
);

static TAutoConsoleVariable<FString> CVarMobileResolutionQualityLimits
(
    TEXT("OT.DeviceProfile.Mobile.ResolutionQualityLimits"),
    TEXT(""),
    TEXT("List of limits on resolution quality of the form \"FPS:MaxResQuality,FPS2:MaxResQuality2,...\", kicking in when FPS is at or above the threshold"),
    ECVF_Default | ECVF_Preview
);

static TAutoConsoleVariable<FString> CVarMobileResolutionQualityRecommendation
(
    TEXT("OT.DeviceProfile.Mobile.ResolutionQualityRecommendation"),
    TEXT("0:75"),
    TEXT("List of limits on resolution quality of the form \"FPS:Recommendation,FPS2:Recommendation2,...\", kicking in when FPS is at or above the threshold"),
    ECVF_Default | ECVF_Preview
);

//////////////////////////////////////////////////////////////////////

FGameScalabilitySnapshot::FGameScalabilitySnapshot()
{
    static_assert(sizeof(Scalability::FQualityLevels) == 88, "This function may need to be updated to account for new members");

    Qualities.ResolutionQuality = -1.0f;
    Qualities.ViewDistanceQuality = -1;
    Qualities.AntiAliasingQuality = -1;
    Qualities.ShadowQuality = -1;
    Qualities.GlobalIlluminationQuality = -1;
    Qualities.ReflectionQuality = -1;
    Qualities.PostProcessQuality = -1;
    Qualities.TextureQuality = -1;
    Qualities.EffectsQuality = -1;
    Qualities.FoliageQuality = -1;
    Qualities.ShadingQuality = -1;
}

//////////////////////////////////////////////////////////////////////

template <typename T>
struct TMobileQualityWrapper
{
private:
    T DefaultValue;
    TAutoConsoleVariable<FString>& WatchedVar;
    FString LastSeenCVarString;

    struct FLimitPair
    {
        int32 Limit = 0;
        T Value = T(0);
    };

    TArray<FLimitPair> Thresholds;

public:
    TMobileQualityWrapper(T InDefaultValue, TAutoConsoleVariable<FString>& InWatchedVar)
        : DefaultValue(InDefaultValue)
      , WatchedVar(InWatchedVar)
    {}

    T Query(int32 TestValue)
    {
        UpdateCache();

        for (const FLimitPair& Pair : Thresholds)
        {
            if (TestValue >= Pair.Limit)
            {
                return Pair.Value;
            }
        }

        return DefaultValue;
    }

    // Returns the first threshold value or INDEX_NONE if there aren't any
    int32 GetFirstThreshold()
    {
        UpdateCache();
        return (Thresholds.Num() > 0) ? Thresholds[0].Limit : INDEX_NONE;
    }

    // Returns the lowest value of all the pairs or DefaultIfNoPairs if there are no pairs
    T GetLowestValue(T DefaultIfNoPairs)
    {
        UpdateCache();

        T Result = DefaultIfNoPairs;
        bool bFirstValue = true;
        for (const FLimitPair& Pair : Thresholds)
        {
            if (bFirstValue)
            {
                Result = Pair.Value;
                bFirstValue = false;
            }
            else
            {
                Result = FMath::Min(Result, Pair.Value);
            }
        }

        return Result;
    }

private:
    void UpdateCache()
    {
        const FString CurrentValue = WatchedVar.GetValueOnGameThread();
        if (!CurrentValue.Equals(LastSeenCVarString, ESearchCase::CaseSensitive))
        {
            LastSeenCVarString = CurrentValue;

            Thresholds.Reset();

            // Parse the thresholds
            int32 ScanIndex = 0;
            while (ScanIndex < LastSeenCVarString.Len())
            {
                const int32 ColonIndex = LastSeenCVarString.Find(TEXT(":"), ESearchCase::CaseSensitive, ESearchDir::FromStart, ScanIndex);
                if (ColonIndex > 0)
                {
                    const int32 CommaIndex = LastSeenCVarString.Find(TEXT(","), ESearchCase::CaseSensitive, ESearchDir::FromStart, ColonIndex);
                    const int32 EndOfPairIndex = (CommaIndex != INDEX_NONE) ? CommaIndex : LastSeenCVarString.Len();

                    FLimitPair Pair;
                    LexFromString(Pair.Limit, *LastSeenCVarString.Mid(ScanIndex, ColonIndex - ScanIndex));
                    LexFromString(Pair.Value, *LastSeenCVarString.Mid(ColonIndex + 1, EndOfPairIndex - ColonIndex - 1));
                    Thresholds.Add(Pair);

                    ScanIndex = EndOfPairIndex + 1;
                }
                else
                {
                    UE_LOG
                    (LogConsoleResponse,
                        Error,
                        TEXT("Malformed value for '%s'='%s', expecting a ':'"),
                        *IConsoleManager::Get().FindConsoleObjectName(WatchedVar.AsVariable()),
                        *LastSeenCVarString);
                    Thresholds.Reset();
                    break;
                }
            }

            // Sort the pairs
            Thresholds.Sort
            ([](const FLimitPair A, const FLimitPair B)
            {
                return A.Limit < B.Limit;
            });
        }
    }
};

namespace GameSettingsHelpers
{
    bool HasPlatformTrait(const FGameplayTag Tag)
    {
        return ICommonUIModule::GetSettings().GetPlatformTraits().HasTag(Tag);
    }

    // Returns the max level from the integer scalability settings (ignores ResolutionQuality)
    int32 GetHighestLevelOfAnyScalabilityChannel(const Scalability::FQualityLevels& ScalabilityQuality)
    {
        static_assert(sizeof(Scalability::FQualityLevels) == 88, "This function may need to be updated to account for new members");

        int32 MaxScalability = ScalabilityQuality.ViewDistanceQuality;
        MaxScalability = FMath::Max(MaxScalability, ScalabilityQuality.AntiAliasingQuality);
        MaxScalability = FMath::Max(MaxScalability, ScalabilityQuality.ShadowQuality);
        MaxScalability = FMath::Max(MaxScalability, ScalabilityQuality.GlobalIlluminationQuality);
        MaxScalability = FMath::Max(MaxScalability, ScalabilityQuality.ReflectionQuality);
        MaxScalability = FMath::Max(MaxScalability, ScalabilityQuality.PostProcessQuality);
        MaxScalability = FMath::Max(MaxScalability, ScalabilityQuality.TextureQuality);
        MaxScalability = FMath::Max(MaxScalability, ScalabilityQuality.EffectsQuality);
        MaxScalability = FMath::Max(MaxScalability, ScalabilityQuality.FoliageQuality);
        MaxScalability = FMath::Max(MaxScalability, ScalabilityQuality.ShadingQuality);

        return (MaxScalability >= 0) ? MaxScalability : -1;
    }

    void FillScalabilitySettingsFromDeviceProfile(FGameScalabilitySnapshot& Mode, const FString& Suffix = FString())
    {
        static_assert(sizeof(Scalability::FQualityLevels) == 88, "This function may need to be updated to account for new members");

        // Default out before filling so we can correctly mark non-overridden scalability values.
        // It's technically possible to swap device profile when testing so safest to clear and refill
        Mode = FGameScalabilitySnapshot();

        Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(FString::Printf(TEXT("sg.ResolutionQuality%s"), *Suffix), Mode.Qualities.ResolutionQuality);
        Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(FString::Printf(TEXT("sg.ViewDistanceQuality%s"), *Suffix), Mode.Qualities.ViewDistanceQuality);
        Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(FString::Printf(TEXT("sg.AntiAliasingQuality%s"), *Suffix), Mode.Qualities.AntiAliasingQuality);
        Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(FString::Printf(TEXT("sg.ShadowQuality%s"), *Suffix), Mode.Qualities.ShadowQuality);
        Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(FString::Printf(TEXT("sg.GlobalIlluminationQuality%s"), *Suffix), Mode.Qualities.GlobalIlluminationQuality);
        Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(FString::Printf(TEXT("sg.ReflectionQuality%s"), *Suffix), Mode.Qualities.ReflectionQuality);
        Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(FString::Printf(TEXT("sg.PostProcessQuality%s"), *Suffix), Mode.Qualities.PostProcessQuality);
        Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(FString::Printf(TEXT("sg.TextureQuality%s"), *Suffix), Mode.Qualities.TextureQuality);
        Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(FString::Printf(TEXT("sg.EffectsQuality%s"), *Suffix), Mode.Qualities.EffectsQuality);
        Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(FString::Printf(TEXT("sg.FoliageQuality%s"), *Suffix), Mode.Qualities.FoliageQuality);
        Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(FString::Printf(TEXT("sg.ShadingQuality%s"), *Suffix), Mode.Qualities.ShadingQuality);
    }

    TMobileQualityWrapper<int32> OverallQualityLimits(-1, CVarMobileQualityLimits);
    TMobileQualityWrapper<float> ResolutionQualityLimits(100.0f, CVarMobileResolutionQualityLimits);
    TMobileQualityWrapper<float> ResolutionQualityRecommendations(75.0f, CVarMobileResolutionQualityRecommendation);

    int32 GetApplicableOverallQualityLimit(const int32 FrameRate)
    {
        return OverallQualityLimits.Query(FrameRate);
    }

    float GetApplicableResolutionQualityLimit(const int32 FrameRate)
    {
        return ResolutionQualityLimits.Query(FrameRate);
    }

    float GetApplicableResolutionQualityRecommendation(const int32 FrameRate)
    {
        return ResolutionQualityRecommendations.Query(FrameRate);
    }

    int32 ConstrainFrameRateToBeCompatibleWithOverallQuality(const int32 FrameRate, const int32 OverallQuality)
    {
        const UUR_PlatformSpecificRenderingSettings* PlatformSettings = UUR_PlatformSpecificRenderingSettings::Get();
        const TArray<int32>& PossibleRates = PlatformSettings->MobileFrameRateLimits;

        // Choose the closest frame rate (without going over) to the user preferred one that is supported and compatible with the desired overall quality
        int32 LimitIndex = PossibleRates.FindLastByPredicate
        ([=](const int32& TestRate)
        {
            const bool bAtOrBelowDesiredRate = (TestRate <= FrameRate);

            const int32 LimitQuality = GetApplicableResolutionQualityLimit(TestRate);
            const bool bQualityDoesntExceedLimit = (LimitQuality < 0) || (OverallQuality <= LimitQuality);

            const bool bIsSupported = UUR_SettingsLocal::IsSupportedMobileFramePace(TestRate);

            return bAtOrBelowDesiredRate && bQualityDoesntExceedLimit && bIsSupported;
        });

        return PossibleRates.IsValidIndex(LimitIndex) ? PossibleRates[LimitIndex] : UUR_SettingsLocal::GetDefaultMobileFrameRate();
    }

    // Returns the first frame rate at which overall quality is restricted/limited by the current device profile
    int32 GetFirstFrameRateWithQualityLimit()
    {
        return OverallQualityLimits.GetFirstThreshold();
    }

    // Returns the lowest quality at which there's a limit on the overall frame rate (or -1 if there is no limit)
    int32 GetLowestQualityWithFrameRateLimit()
    {
        return OverallQualityLimits.GetLowestValue(-1);
    }
}

//////////////////////////////////////////////////////////////////////

PRAGMA_DISABLE_DEPRECATION_WARNINGS

UUR_SettingsLocal::UUR_SettingsLocal()
{
    if (!HasAnyFlags(RF_ClassDefaultObject) && FSlateApplication::IsInitialized())
    {
        OnApplicationActivationStateChangedHandle = FSlateApplication::Get().OnApplicationActivationStateChanged().AddUObject(this, &ThisClass::OnAppActivationStateChanged);
    }

	bEnableScalabilitySettings = UUR_PlatformSpecificRenderingSettings::Get()->bSupportsGranularVideoQualitySettings;

    SetToDefaults();
}

PRAGMA_ENABLE_DEPRECATION_WARNINGS

void UUR_SettingsLocal::SetToDefaults()
{
    Super::SetToDefaults();

    bUseHeadphoneMode = false;
    bUseHDRAudioMode = false;
    bSoundControlBusMixLoaded = false;

    const UUR_PlatformSpecificRenderingSettings* PlatformSettings = UUR_PlatformSpecificRenderingSettings::Get();
    UserChosenDeviceProfileSuffix = PlatformSettings->DefaultDeviceProfileSuffix;
    DesiredUserChosenDeviceProfileSuffix = UserChosenDeviceProfileSuffix;

    FrameRateLimit_InMenu = 144.0f;
    FrameRateLimit_WhenBackgrounded = 30.0f;
    FrameRateLimit_OnBattery = 60.0f;

    MobileFrameRateLimit = GetDefaultMobileFrameRate();
    DesiredMobileFrameRateLimit = MobileFrameRateLimit;
}

void UUR_SettingsLocal::LoadSettings(bool bForceReload)
{
    Super::LoadSettings(bForceReload);

    // Console platforms use rhi.SyncInterval to limit framerate
    const UUR_PlatformSpecificRenderingSettings* PlatformSettings = UUR_PlatformSpecificRenderingSettings::Get();
    if (PlatformSettings->FramePacingMode == EGameFramePacingMode::ConsoleStyle)
    {
        FrameRateLimit = 0.0f;
    }

    // Enable HRTF if needed
    bDesiredHeadphoneMode = bUseHeadphoneMode;
    SetHeadphoneModeEnabled(bUseHeadphoneMode);


    DesiredUserChosenDeviceProfileSuffix = UserChosenDeviceProfileSuffix;

    GameSettingsHelpers::FillScalabilitySettingsFromDeviceProfile(DeviceDefaultScalabilitySettings);

    DesiredMobileFrameRateLimit = MobileFrameRateLimit;
    ClampMobileQuality();


    PerfStatSettingsChangedEvent.Broadcast();
}

void UUR_SettingsLocal::ResetToCurrentSettings()
{
    Super::ResetToCurrentSettings();

    bDesiredHeadphoneMode = bUseHeadphoneMode;

    UserChosenDeviceProfileSuffix = DesiredUserChosenDeviceProfileSuffix;

    MobileFrameRateLimit = DesiredMobileFrameRateLimit;
}

void UUR_SettingsLocal::BeginDestroy()
{
    if (FSlateApplication::IsInitialized())
    {
        FSlateApplication::Get().OnApplicationActivationStateChanged().Remove(OnApplicationActivationStateChangedHandle);
    }

    Super::BeginDestroy();
}

UUR_SettingsLocal* UUR_SettingsLocal::Get()
{
    return GEngine ? CastChecked<UUR_SettingsLocal>(GEngine->GetGameUserSettings()) : nullptr;
}

void UUR_SettingsLocal::ConfirmVideoMode()
{
    Super::ConfirmVideoMode();

    SetMobileFPSMode(DesiredMobileFrameRateLimit);
}

// Combines two limits, always taking the minimum of the two (with special handling for values of <= 0 meaning unlimited)
float CombineFrameRateLimits(const float Limit1, const float Limit2)
{
    if (Limit1 <= 0.0f)
    {
        return Limit2;
    }
    else if (Limit2 <= 0.0f)
    {
        return Limit1;
    }
    else
    {
        return FMath::Min(Limit1, Limit2);
    }
}

float UUR_SettingsLocal::GetEffectiveFrameRateLimit()
{
    const UUR_PlatformSpecificRenderingSettings* PlatformSettings = UUR_PlatformSpecificRenderingSettings::Get();

#if WITH_EDITOR
    if (GIsEditor && !CVarApplyFrameRateSettingsInPIE.GetValueOnGameThread())
    {
        return Super::GetEffectiveFrameRateLimit();
    }
#endif

    if (PlatformSettings->FramePacingMode == EGameFramePacingMode::ConsoleStyle)
    {
        return 0.0f;
    }

    float EffectiveFrameRateLimit = Super::GetEffectiveFrameRateLimit();

    if (ShouldUseFrontendPerformanceSettings())
    {
        EffectiveFrameRateLimit = CombineFrameRateLimits(EffectiveFrameRateLimit, FrameRateLimit_InMenu);
    }

    if (PlatformSettings->FramePacingMode == EGameFramePacingMode::DesktopStyle)
    {
        if (FPlatformMisc::IsRunningOnBattery())
        {
            EffectiveFrameRateLimit = CombineFrameRateLimits(EffectiveFrameRateLimit, FrameRateLimit_OnBattery);
        }

        if (FSlateApplication::IsInitialized() && !FSlateApplication::Get().IsActive())
        {
            EffectiveFrameRateLimit = CombineFrameRateLimits(EffectiveFrameRateLimit, FrameRateLimit_WhenBackgrounded);
        }
    }

    return EffectiveFrameRateLimit;
}

int32 UUR_SettingsLocal::GetHighestLevelOfAnyScalabilityChannel() const
{
    return GameSettingsHelpers::GetHighestLevelOfAnyScalabilityChannel(ScalabilityQuality);
}

void UUR_SettingsLocal::OverrideQualityLevelsToScalabilityMode(const FGameScalabilitySnapshot& InMode, Scalability::FQualityLevels& InOutLevels)
{
    static_assert(sizeof(Scalability::FQualityLevels) == 88, "This function may need to be updated to account for new members");

    // Overrides any valid (non-negative) settings
    InOutLevels.ResolutionQuality = (InMode.Qualities.ResolutionQuality >= 0.f) ? InMode.Qualities.ResolutionQuality : InOutLevels.ResolutionQuality;
    InOutLevels.ViewDistanceQuality = (InMode.Qualities.ViewDistanceQuality >= 0) ? InMode.Qualities.ViewDistanceQuality : InOutLevels.ViewDistanceQuality;
    InOutLevels.AntiAliasingQuality = (InMode.Qualities.AntiAliasingQuality >= 0) ? InMode.Qualities.AntiAliasingQuality : InOutLevels.AntiAliasingQuality;
    InOutLevels.ShadowQuality = (InMode.Qualities.ShadowQuality >= 0) ? InMode.Qualities.ShadowQuality : InOutLevels.ShadowQuality;
    InOutLevels.GlobalIlluminationQuality = (InMode.Qualities.GlobalIlluminationQuality >= 0) ? InMode.Qualities.GlobalIlluminationQuality : InOutLevels.GlobalIlluminationQuality;
    InOutLevels.ReflectionQuality = (InMode.Qualities.ReflectionQuality >= 0) ? InMode.Qualities.ReflectionQuality : InOutLevels.ReflectionQuality;
    InOutLevels.PostProcessQuality = (InMode.Qualities.PostProcessQuality >= 0) ? InMode.Qualities.PostProcessQuality : InOutLevels.PostProcessQuality;
    InOutLevels.TextureQuality = (InMode.Qualities.TextureQuality >= 0) ? InMode.Qualities.TextureQuality : InOutLevels.TextureQuality;
    InOutLevels.EffectsQuality = (InMode.Qualities.EffectsQuality >= 0) ? InMode.Qualities.EffectsQuality : InOutLevels.EffectsQuality;
    InOutLevels.FoliageQuality = (InMode.Qualities.FoliageQuality >= 0) ? InMode.Qualities.FoliageQuality : InOutLevels.FoliageQuality;
    InOutLevels.ShadingQuality = (InMode.Qualities.ShadingQuality >= 0) ? InMode.Qualities.ShadingQuality : InOutLevels.ShadingQuality;
}

void UUR_SettingsLocal::ClampQualityLevelsToDeviceProfile(const Scalability::FQualityLevels& ClampLevels, Scalability::FQualityLevels& InOutLevels)
{
    static_assert(sizeof(Scalability::FQualityLevels) == 88, "This function may need to be updated to account for new members");

    // Clamps any valid (non-negative) settings
    InOutLevels.ResolutionQuality = (ClampLevels.ResolutionQuality >= 0.f) ? FMath::Min(ClampLevels.ResolutionQuality, InOutLevels.ResolutionQuality) : InOutLevels.ResolutionQuality;
    InOutLevels.ViewDistanceQuality = (ClampLevels.ViewDistanceQuality >= 0) ? FMath::Min(ClampLevels.ViewDistanceQuality, InOutLevels.ViewDistanceQuality) : InOutLevels.ViewDistanceQuality;
    InOutLevels.AntiAliasingQuality = (ClampLevels.AntiAliasingQuality >= 0) ? FMath::Min(ClampLevels.AntiAliasingQuality, InOutLevels.AntiAliasingQuality) : InOutLevels.AntiAliasingQuality;
    InOutLevels.ShadowQuality = (ClampLevels.ShadowQuality >= 0) ? FMath::Min(ClampLevels.ShadowQuality, InOutLevels.ShadowQuality) : InOutLevels.ShadowQuality;
    InOutLevels.GlobalIlluminationQuality = (ClampLevels.GlobalIlluminationQuality >= 0) ? FMath::Min(ClampLevels.GlobalIlluminationQuality, InOutLevels.GlobalIlluminationQuality) : InOutLevels.GlobalIlluminationQuality;
    InOutLevels.ReflectionQuality = (ClampLevels.ReflectionQuality >= 0) ? FMath::Min(ClampLevels.ReflectionQuality, InOutLevels.ReflectionQuality) : InOutLevels.ReflectionQuality;
    InOutLevels.PostProcessQuality = (ClampLevels.PostProcessQuality >= 0) ? FMath::Min(ClampLevels.PostProcessQuality, InOutLevels.PostProcessQuality) : InOutLevels.PostProcessQuality;
    InOutLevels.TextureQuality = (ClampLevels.TextureQuality >= 0) ? FMath::Min(ClampLevels.TextureQuality, InOutLevels.TextureQuality) : InOutLevels.TextureQuality;
    InOutLevels.EffectsQuality = (ClampLevels.EffectsQuality >= 0) ? FMath::Min(ClampLevels.EffectsQuality, InOutLevels.EffectsQuality) : InOutLevels.EffectsQuality;
    InOutLevels.FoliageQuality = (ClampLevels.FoliageQuality >= 0) ? FMath::Min(ClampLevels.FoliageQuality, InOutLevels.FoliageQuality) : InOutLevels.FoliageQuality;
    InOutLevels.ShadingQuality = (ClampLevels.ShadingQuality >= 0) ? FMath::Min(ClampLevels.ShadingQuality, InOutLevels.ShadingQuality) : InOutLevels.ShadingQuality;
}

void UUR_SettingsLocal::OnExperienceLoaded()
{
    ReapplyThingsDueToPossibleDeviceProfileChange();
}

void UUR_SettingsLocal::OnHotfixDeviceProfileApplied()
{
    ReapplyThingsDueToPossibleDeviceProfileChange();
}

void UUR_SettingsLocal::ReapplyThingsDueToPossibleDeviceProfileChange()
{
    ApplyNonResolutionSettings();
}

void UUR_SettingsLocal::SetShouldUseFrontendPerformanceSettings(bool bInFrontEnd)
{
    bInFrontEndForPerformancePurposes = bInFrontEnd;
    UpdateEffectiveFrameRateLimit();
}

bool UUR_SettingsLocal::ShouldUseFrontendPerformanceSettings() const
{
#if WITH_EDITOR
    if (GIsEditor && !CVarApplyFrontEndPerformanceOptionsInPIE.GetValueOnGameThread())
    {
        return false;
    }
#endif

    return bInFrontEndForPerformancePurposes;
}

EGameStatDisplayMode UUR_SettingsLocal::GetPerfStatDisplayState(EGameDisplayablePerformanceStat Stat) const
{
    if (const EGameStatDisplayMode* PMode = DisplayStatList.Find(Stat))
    {
        return *PMode;
    }
    else
    {
        return EGameStatDisplayMode::Hidden;
    }
}

void UUR_SettingsLocal::SetPerfStatDisplayState(EGameDisplayablePerformanceStat Stat, EGameStatDisplayMode DisplayMode)
{
    if (DisplayMode == EGameStatDisplayMode::Hidden)
    {
        DisplayStatList.Remove(Stat);
    }
    else
    {
        DisplayStatList.FindOrAdd(Stat) = DisplayMode;
    }
    PerfStatSettingsChangedEvent.Broadcast();
}

bool UUR_SettingsLocal::DoesPlatformSupportLatencyMarkers()
{
    return ICommonUIModule::GetSettings().GetPlatformTraits().HasTag(PerfStatTags::TAG_Platform_Trait_SupportsLatencyMarkers);
}

void UUR_SettingsLocal::SetEnableLatencyFlashIndicators(const bool bNewVal)
{
    if (bNewVal != bEnableLatencyFlashIndicators)
    {
        bEnableLatencyFlashIndicators = bNewVal;
        LatencyFlashIndicatorSettingsChangedEvent.Broadcast();
    }
}

void UUR_SettingsLocal::SetEnableLatencyTrackingStats(const bool bNewVal)
{
    if (bNewVal != bEnableLatencyTrackingStats)
    {
        bEnableLatencyTrackingStats = bNewVal;

        ApplyLatencyTrackingStatSetting();

        LatencyStatIndicatorSettingsChangedEvent.Broadcast();
    }
}

void UUR_SettingsLocal::ApplyLatencyTrackingStatSetting()
{
    // Since this function will be called on load of the settings, we check if the slate app is initialized.
    // If it isn't then we are not in a target which can even have latency stats (like a headless cooker) so we
    // will exit early and do nothing.
    if (!FSlateApplication::IsInitialized())
    {
        return;
    }

    // Don't bother doing anything if the platform doesn't even support tracking stats.
    if (!DoesPlatformSupportLatencyTrackingStats())
    {
        return;
    }

    // Actually enable or disable the latency marker modules based on this setting
    TArray<ILatencyMarkerModule*> LatencyMarkerModules = IModularFeatures::Get().GetModularFeatureImplementations<ILatencyMarkerModule>(ILatencyMarkerModule::GetModularFeatureName());
    for (ILatencyMarkerModule* LatencyMarkerModule : LatencyMarkerModules)
    {
        LatencyMarkerModule->SetEnabled(bEnableLatencyTrackingStats);
    }

    UE_CLOG(!LatencyMarkerModules.IsEmpty(),
        LogConsoleResponse,
        Log,
        TEXT("%s %d Latency Marker Module(s)"),
        bEnableLatencyTrackingStats ? TEXT("Enabled") : TEXT("Disabled"), LatencyMarkerModules.Num());
}

bool UUR_SettingsLocal::DoesPlatformSupportLatencyTrackingStats()
{
    return ICommonUIModule::GetSettings().GetPlatformTraits().HasTag(PerfStatTags::TAG_Platform_Trait_SupportsLatencyStats);
}

float UUR_SettingsLocal::GetDisplayGamma() const
{
    return DisplayGamma;
}

void UUR_SettingsLocal::SetDisplayGamma(float InGamma)
{
    DisplayGamma = InGamma;
    ApplyDisplayGamma();
}

void UUR_SettingsLocal::ApplyDisplayGamma()
{
    if (GEngine)
    {
        GEngine->DisplayGamma = DisplayGamma;
    }
}

float UUR_SettingsLocal::GetFrameRateLimit_OnBattery() const
{
    return FrameRateLimit_OnBattery;
}

void UUR_SettingsLocal::SetFrameRateLimit_OnBattery(float NewLimitFPS)
{
    FrameRateLimit_OnBattery = NewLimitFPS;
    UpdateEffectiveFrameRateLimit();
}

float UUR_SettingsLocal::GetFrameRateLimit_InMenu() const
{
    return FrameRateLimit_InMenu;
}

void UUR_SettingsLocal::SetFrameRateLimit_InMenu(float NewLimitFPS)
{
    FrameRateLimit_InMenu = NewLimitFPS;
    UpdateEffectiveFrameRateLimit();
}

float UUR_SettingsLocal::GetFrameRateLimit_WhenBackgrounded() const
{
    return FrameRateLimit_WhenBackgrounded;
}

void UUR_SettingsLocal::SetFrameRateLimit_WhenBackgrounded(float NewLimitFPS)
{
    FrameRateLimit_WhenBackgrounded = NewLimitFPS;
    UpdateEffectiveFrameRateLimit();
}

float UUR_SettingsLocal::GetFrameRateLimit_Always() const
{
    return GetFrameRateLimit();
}

void UUR_SettingsLocal::SetFrameRateLimit_Always(float NewLimitFPS)
{
    SetFrameRateLimit(NewLimitFPS);
    UpdateEffectiveFrameRateLimit();
}

void UUR_SettingsLocal::UpdateEffectiveFrameRateLimit()
{
    if (!IsRunningDedicatedServer())
    {
        SetFrameRateLimitCVar(GetEffectiveFrameRateLimit());
    }
}

int32 UUR_SettingsLocal::GetDefaultMobileFrameRate()
{
    return CVarDeviceProfileDrivenMobileDefaultFrameRate.GetValueOnGameThread();
}

int32 UUR_SettingsLocal::GetMaxMobileFrameRate()
{
    return CVarDeviceProfileDrivenMobileMaxFrameRate.GetValueOnGameThread();
}

bool UUR_SettingsLocal::IsSupportedMobileFramePace(int32 TestFPS)
{
    const bool bIsDefault = (TestFPS == GetDefaultMobileFrameRate());
    const bool bDoesNotExceedLimit = (TestFPS <= GetMaxMobileFrameRate());

    // Allow all paces in the editor, as we'd only be doing this when simulating another platform
    const bool bIsSupportedPace = FPlatformRHIFramePacer::SupportsFramePace(TestFPS) || GIsEditor;

    return bIsDefault || (bDoesNotExceedLimit && bIsSupportedPace);
}

int32 UUR_SettingsLocal::GetFirstFrameRateWithQualityLimit() const
{
    return GameSettingsHelpers::GetFirstFrameRateWithQualityLimit();
}

int32 UUR_SettingsLocal::GetLowestQualityWithFrameRateLimit() const
{
    return GameSettingsHelpers::GetLowestQualityWithFrameRateLimit();
}

void UUR_SettingsLocal::ResetToMobileDeviceDefaults()
{
    // Reset frame rate
    DesiredMobileFrameRateLimit = GetDefaultMobileFrameRate();
    MobileFrameRateLimit = DesiredMobileFrameRateLimit;

    // Reset scalability
    Scalability::FQualityLevels DefaultLevels = Scalability::GetQualityLevels();
    OverrideQualityLevelsToScalabilityMode(DeviceDefaultScalabilitySettings, DefaultLevels);
    ScalabilityQuality = DefaultLevels;

    // Apply
    UpdateGameModeDeviceProfileAndFps();
}

int32 UUR_SettingsLocal::GetMaxSupportedOverallQualityLevel() const
{
    const UUR_PlatformSpecificRenderingSettings* PlatformSettings = UUR_PlatformSpecificRenderingSettings::Get();
    if ((PlatformSettings->FramePacingMode == EGameFramePacingMode::MobileStyle) && DeviceDefaultScalabilitySettings.bHasOverrides)
    {
        return GameSettingsHelpers::GetHighestLevelOfAnyScalabilityChannel(DeviceDefaultScalabilitySettings.Qualities);
    }
    else
    {
        return 3;
    }
}

void UUR_SettingsLocal::SetMobileFPSMode(const int32 NewLimitFPS)
{
    const UUR_PlatformSpecificRenderingSettings* PlatformSettings = UUR_PlatformSpecificRenderingSettings::Get();
    if (PlatformSettings->FramePacingMode == EGameFramePacingMode::MobileStyle)
    {
        if (MobileFrameRateLimit != NewLimitFPS)
        {
            MobileFrameRateLimit = NewLimitFPS;
            UpdateGameModeDeviceProfileAndFps();
        }

        DesiredMobileFrameRateLimit = MobileFrameRateLimit;
    }
}

void UUR_SettingsLocal::SetDesiredMobileFrameRateLimit(const int32 NewLimitFPS)
{
    const int32 OldLimitFPS = DesiredMobileFrameRateLimit;

    RemapMobileResolutionQuality(OldLimitFPS, NewLimitFPS);

    DesiredMobileFrameRateLimit = NewLimitFPS;

    ClampMobileFPSQualityLevels(/*bWriteBack=*/ false);
}

void UUR_SettingsLocal::ClampMobileFPSQualityLevels(bool bWriteBack)
{
    const UUR_PlatformSpecificRenderingSettings* PlatformSettings = UUR_PlatformSpecificRenderingSettings::Get();
    if (PlatformSettings->FramePacingMode == EGameFramePacingMode::MobileStyle)
    {
        const int32 QualityLimit = GameSettingsHelpers::GetApplicableOverallQualityLimit(DesiredMobileFrameRateLimit);
        const int32 CurrentQualityLevel = GetHighestLevelOfAnyScalabilityChannel();
        if ((QualityLimit >= 0) && (CurrentQualityLevel > QualityLimit))
        {
            SetOverallScalabilityLevel(QualityLimit);

            if (bWriteBack)
            {
                Scalability::SetQualityLevels(ScalabilityQuality);
            }
            UE_LOG(LogConsoleResponse, Log, TEXT("Mobile FPS clamped overall quality (%d -> %d)."), CurrentQualityLevel, QualityLimit);
        }
    }
}

void UUR_SettingsLocal::ClampMobileQuality()
{
    const UUR_PlatformSpecificRenderingSettings* PlatformSettings = UUR_PlatformSpecificRenderingSettings::Get();
    if (PlatformSettings->FramePacingMode == EGameFramePacingMode::MobileStyle)
    {
        // Clamp the resultant settings to the device default, it's known viable maximum.
        // This is a clamp rather than override to preserve allowed user settings
        Scalability::FQualityLevels CurrentLevels = Scalability::GetQualityLevels();

        /** On mobile, disables the 3D Resolution clamp that reverts the setting set by the user on boot.*/
        bool bMobileDisableResolutionReset = true;
        if (bMobileDisableResolutionReset)
        {
            DeviceDefaultScalabilitySettings.Qualities.ResolutionQuality = CurrentLevels.ResolutionQuality;
        }

        ClampQualityLevelsToDeviceProfile(DeviceDefaultScalabilitySettings.Qualities, /*inout*/ CurrentLevels);
        Scalability::SetQualityLevels(CurrentLevels);

        // Clamp quality levels if required at the current frame rate
        ClampMobileFPSQualityLevels(/*bWriteBack=*/ true);

        const int32 MaxMobileFrameRate = GetMaxMobileFrameRate();
        const int32 DefaultMobileFrameRate = GetDefaultMobileFrameRate();

        ensureMsgf(DefaultMobileFrameRate <= MaxMobileFrameRate, TEXT("Default mobile frame rate (%d) is higher than the maximum mobile frame rate (%d)!"), DefaultMobileFrameRate, MaxMobileFrameRate);

        // Choose the closest supported frame rate to the user desired setting without going over the device imposed limit
        const TArray<int32>& PossibleRates = PlatformSettings->MobileFrameRateLimits;
        const int32 LimitIndex = PossibleRates.FindLastByPredicate
        ([this](const int32& TestRate)
        {
            return (TestRate <= DesiredMobileFrameRateLimit) && IsSupportedMobileFramePace(TestRate);
        });
        const int32 ActualLimitFPS = PossibleRates.IsValidIndex(LimitIndex) ? PossibleRates[LimitIndex] : GetDefaultMobileFrameRate();

        ClampMobileResolutionQuality(ActualLimitFPS);
    }
}

void UUR_SettingsLocal::ClampMobileResolutionQuality(int32 TargetFPS)
{
    // Clamp mobile resolution quality
    const float MaxMobileResQuality = GameSettingsHelpers::GetApplicableResolutionQualityLimit(TargetFPS);
    float CurrentScaleNormalized = 0.0f;
    float CurrentScaleValue = 0.0f;
    float MinScaleValue = 0.0f;
    float MaxScaleValue = 0.0f;
    GetResolutionScaleInformationEx(CurrentScaleNormalized, CurrentScaleValue, MinScaleValue, MaxScaleValue);
    if (CurrentScaleValue > MaxMobileResQuality)
    {
        UE_LOG(LogConsoleResponse, Log, TEXT("clamping mobile resolution quality max res: %f, %f, %f, %f, %f"), CurrentScaleNormalized, CurrentScaleValue, MinScaleValue, MaxScaleValue, MaxMobileResQuality);
        SetResolutionScaleValueEx(MaxMobileResQuality);
    }
}

void UUR_SettingsLocal::RemapMobileResolutionQuality(const int32 FromFPS, const int32 ToFPS)
{
    // Mobile resolution quality slider is a normalized value that is lerped between min quality, max quality.
    // max quality can change depending on FPS mode. This code remaps the quality when FPS mode changes so that the normalized
    // value remains the same within the new range.
    float CurrentScaleNormalized = 0.0f;
    float CurrentScaleValue = 0.0f;
    float MinScaleValue = 0.0f;
    float MaxScaleValue = 0.0f;
    GetResolutionScaleInformationEx(CurrentScaleNormalized, CurrentScaleValue, MinScaleValue, MaxScaleValue);
    const float FromMaxMobileResQuality = GameSettingsHelpers::GetApplicableResolutionQualityLimit(FromFPS);
    const float ToMaxMobileResQuality = GameSettingsHelpers::GetApplicableResolutionQualityLimit(ToFPS);
    const float FromMobileScaledNormalizedValue = (CurrentScaleValue - MinScaleValue) / (FromMaxMobileResQuality - MinScaleValue);
    const float ToResQuality = FMath::Lerp(MinScaleValue, ToMaxMobileResQuality, FromMobileScaledNormalizedValue);

    UE_LOG(LogConsoleResponse, Log, TEXT("Remap mobile resolution quality %f, %f, (%d,%d)"), CurrentScaleValue, ToResQuality, FromFPS, ToFPS);

    SetResolutionScaleValueEx(ToResQuality);
}

FString UUR_SettingsLocal::GetDesiredDeviceProfileQualitySuffix() const
{
    return DesiredUserChosenDeviceProfileSuffix;
}

void UUR_SettingsLocal::SetDesiredDeviceProfileQualitySuffix(const FString& InDesiredSuffix)
{
    DesiredUserChosenDeviceProfileSuffix = InDesiredSuffix;
}

void UUR_SettingsLocal::SetHeadphoneModeEnabled(bool bEnabled)
{
    if (CanModifyHeadphoneModeEnabled())
    {
        static IConsoleVariable* BinauralSpatializationDisabledCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("au.DisableBinauralSpatialization"));
        if (BinauralSpatializationDisabledCVar)
        {
            BinauralSpatializationDisabledCVar->Set(!bEnabled, ECVF_SetByGameSetting);

            // Only save settings if the setting actually changed
            if (bUseHeadphoneMode != bEnabled)
            {
                bUseHeadphoneMode = bEnabled;
                SaveSettings();
            }
        }
    }
}

bool UUR_SettingsLocal::IsHeadphoneModeEnabled() const
{
    return bUseHeadphoneMode;
}

bool UUR_SettingsLocal::CanModifyHeadphoneModeEnabled() const
{
    static IConsoleVariable* BinauralSpatializationDisabledCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("au.DisableBinauralSpatialization"));
    const bool bHRTFOptionAvailable = BinauralSpatializationDisabledCVar && ((BinauralSpatializationDisabledCVar->GetFlags() & EConsoleVariableFlags::ECVF_SetByMask) <= EConsoleVariableFlags::ECVF_SetByGameSetting);

    const bool bBinauralSettingControlledByOS = GameSettingsHelpers::HasPlatformTrait(TAG_Platform_Trait_BinauralSettingControlledByOS);

    return bHRTFOptionAvailable && !bBinauralSettingControlledByOS;
}

bool UUR_SettingsLocal::IsHDRAudioModeEnabled() const
{
    return bUseHDRAudioMode;
}

void UUR_SettingsLocal::SetHDRAudioModeEnabled(const bool bEnabled)
{
    bUseHDRAudioMode = bEnabled;

    if (GEngine)
    {
        if (const UWorld* World = GEngine->GetCurrentPlayWorld())
        {
            if (UUR_AudioMixEffectsSubsystem* GameAudioMixEffectsSubsystem = World->GetSubsystem<UUR_AudioMixEffectsSubsystem>())
            {
                GameAudioMixEffectsSubsystem->ApplyDynamicRangeEffectsChains(bEnabled);
            }
        }
    }
}

bool UUR_SettingsLocal::CanRunAutoBenchmark() const
{
    const UUR_PlatformSpecificRenderingSettings* PlatformSettings = UUR_PlatformSpecificRenderingSettings::Get();
    return PlatformSettings->bSupportsAutomaticVideoQualityBenchmark;
}

bool UUR_SettingsLocal::ShouldRunAutoBenchmarkAtStartup() const
{
    if (!CanRunAutoBenchmark())
    {
        return false;
    }

    if (LastCPUBenchmarkResult != -1)
    {
        // Already run and loaded
        return false;
    }

    return true;
}

void UUR_SettingsLocal::RunAutoBenchmark(const bool bSaveImmediately)
{
    RunHardwareBenchmark();

    // Always apply, optionally save
    ApplyScalabilitySettings();

    if (bSaveImmediately)
    {
        SaveSettings();
    }
}

void UUR_SettingsLocal::ApplyScalabilitySettings()
{
    Scalability::SetQualityLevels(ScalabilityQuality);
}

float UUR_SettingsLocal::GetOverallVolume() const
{
    return OverallVolume;
}

void UUR_SettingsLocal::SetOverallVolume(float InVolume)
{
    // Cache the incoming volume value
    OverallVolume = InVolume;

    // Check to see if references to the control buses and control bus mixes have been loaded yet
    // Will likely need to be loaded if this function is the first time a setter has been called from the UI
    if (!bSoundControlBusMixLoaded)
    {
        LoadUserControlBusMix();
    }

    // Ensure it's been loaded before continuing
    ensureMsgf(bSoundControlBusMixLoaded, TEXT("UserControlBusMix Settings Failed to Load."));

    // Locate the locally cached bus and set the volume on it
    if (TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(TEXT("Overall")))
    {
        if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
        {
            SetVolumeForControlBus(ControlBusPtr, OverallVolume);
        }
    }
}

float UUR_SettingsLocal::GetMusicVolume() const
{
    return MusicVolume;
}

void UUR_SettingsLocal::SetMusicVolume(const float InVolume)
{
    // Cache the incoming volume value
    MusicVolume = InVolume;

    // Check to see if references to the control buses and control bus mixes have been loaded yet
    // Will likely need to be loaded if this function is the first time a setter has been called from the UI
    if (!bSoundControlBusMixLoaded)
    {
        LoadUserControlBusMix();
    }

    // Ensure it's been loaded before continuing
    ensureMsgf(bSoundControlBusMixLoaded, TEXT("UserControlBusMix Settings Failed to Load."));

    // Locate the locally cached bus and set the volume on it
    if (const TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(TEXT("Music")))
    {
        if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
        {
            SetVolumeForControlBus(ControlBusPtr, MusicVolume);
        }
    }
}

float UUR_SettingsLocal::GetSoundFXVolume() const
{
    return SoundFXVolume;
}

void UUR_SettingsLocal::SetSoundFXVolume(const float InVolume)
{
    // Cache the incoming volume value
    SoundFXVolume = InVolume;

    // Check to see if references to the control buses and control bus mixes have been loaded yet
    // Will likely need to be loaded if this function is the first time a setter has been called from the UI
    if (!bSoundControlBusMixLoaded)
    {
        LoadUserControlBusMix();
    }

    // Ensure it's been loaded before continuing
    ensureMsgf(bSoundControlBusMixLoaded, TEXT("UserControlBusMix Settings Failed to Load."));

    // Locate the locally cached bus and set the volume on it
    if (const TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(TEXT("SoundFX")))
    {
        if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
        {
            SetVolumeForControlBus(ControlBusPtr, SoundFXVolume);
        }
    }
}

float UUR_SettingsLocal::GetDialogueVolume() const
{
    return DialogueVolume;
}

void UUR_SettingsLocal::SetDialogueVolume(const float InVolume)
{
    // Cache the incoming volume value
    DialogueVolume = InVolume;

    // Check to see if references to the control buses and control bus mixes have been loaded yet
    // Will likely need to be loaded if this function is the first time a setter has been called from the UI
    if (!bSoundControlBusMixLoaded)
    {
        LoadUserControlBusMix();
    }

    // Ensure it's been loaded before continuing
    ensureMsgf(bSoundControlBusMixLoaded, TEXT("UserControlBusMix Settings Failed to Load."));

    // Locate the locally cached bus and set the volume on it
    if (const TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(TEXT("Dialogue")))
    {
        if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
        {
            SetVolumeForControlBus(ControlBusPtr, DialogueVolume);
        }
    }
}

float UUR_SettingsLocal::GetVoiceChatVolume() const
{
    return VoiceChatVolume;
}

void UUR_SettingsLocal::SetVoiceChatVolume(const float InVolume)
{
    // Cache the incoming volume value
    VoiceChatVolume = InVolume;

    // Check to see if references to the control buses and control bus mixes have been loaded yet
    // Will likely need to be loaded if this function is the first time a setter has been called from the UI
    if (!bSoundControlBusMixLoaded)
    {
        LoadUserControlBusMix();
    }

    // Ensure it's been loaded before continuing
    ensureMsgf(bSoundControlBusMixLoaded, TEXT("UserControlBusMix Settings Failed to Load."));

    // Locate the locally cached bus and set the volume on it
    if (TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(TEXT("VoiceChat")))
    {
        if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
        {
            SetVolumeForControlBus(ControlBusPtr, VoiceChatVolume);
        }
    }
}

void UUR_SettingsLocal::SetVolumeForControlBus(USoundControlBus* InSoundControlBus, const float InVolume)
{
    // Check to see if references to the control buses and control bus mixes have been loaded yet
    // Will likely need to be loaded if this function is the first time a setter has been called
    if (!bSoundControlBusMixLoaded)
    {
        LoadUserControlBusMix();
    }

    // Ensure it's been loaded before continuing
    ensureMsgf(bSoundControlBusMixLoaded, TEXT("UserControlBusMix Settings Failed to Load."));

    // Assuming everything has been loaded correctly, we retrieve the world and use AudioModulationStatics to update the Control Bus Volume values and
    // apply the settings to the cached User Control Bus Mix
    if (GEngine && InSoundControlBus && bSoundControlBusMixLoaded)
    {
        if (const UWorld* AudioWorld = GEngine->GetCurrentPlayWorld())
        {
            ensureMsgf(ControlBusMix, TEXT("Control Bus Mix failed to load."));

            // Create and set the Control Bus Mix Stage Parameters
            FSoundControlBusMixStage UpdatedControlBusMixStage;
            UpdatedControlBusMixStage.Bus = InSoundControlBus;
            UpdatedControlBusMixStage.Value.TargetValue = InVolume;
            UpdatedControlBusMixStage.Value.AttackTime = 0.01f;
            UpdatedControlBusMixStage.Value.ReleaseTime = 0.01f;

            // Add the Control Bus Mix Stage to an Array as the UpdateMix function requires
            TArray<FSoundControlBusMixStage> UpdatedMixStageArray;
            UpdatedMixStageArray.Add(UpdatedControlBusMixStage);

            // Modify the matching bus Mix Stage parameters on the User Control Bus Mix
            UAudioModulationStatics::UpdateMix(AudioWorld, ControlBusMix, UpdatedMixStageArray);
        }
    }
}

void UUR_SettingsLocal::SetAudioOutputDeviceId(const FString& InAudioOutputDeviceId)
{
    AudioOutputDeviceId = InAudioOutputDeviceId;
    OnAudioOutputDeviceChanged.Broadcast(InAudioOutputDeviceId);
}

void UUR_SettingsLocal::ApplySafeZoneScale()
{
    SSafeZone::SetGlobalSafeZoneScale(GetSafeZone());
}

void UUR_SettingsLocal::ApplyNonResolutionSettings()
{
    Super::ApplyNonResolutionSettings();

    // Check if Control Bus Mix references have been loaded,
    // Might be false if applying non resolution settings without touching any of the setters from UI
    if (!bSoundControlBusMixLoaded)
    {
        LoadUserControlBusMix();
    }

    // In this section, update each Control Bus to the currently cached UI settings
    {
        if (const TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(TEXT("Overall")))
        {
            if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
            {
                SetVolumeForControlBus(ControlBusPtr, OverallVolume);
            }
        }

        if (const TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(TEXT("Music")))
        {
            if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
            {
                SetVolumeForControlBus(ControlBusPtr, MusicVolume);
            }
        }

        if (const TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(TEXT("SoundFX")))
        {
            if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
            {
                SetVolumeForControlBus(ControlBusPtr, SoundFXVolume);
            }
        }

        if (const TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(TEXT("Dialogue")))
        {
            if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
            {
                SetVolumeForControlBus(ControlBusPtr, DialogueVolume);
            }
        }

        if (const TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(TEXT("VoiceChat")))
        {
            if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
            {
                SetVolumeForControlBus(ControlBusPtr, VoiceChatVolume);
            }
        }
    }

    if (UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetTypedOuter<ULocalPlayer>()))
    {
        InputSubsystem->SetGamepadInputType(ControllerPlatform);
    }

    if (bUseHeadphoneMode != bDesiredHeadphoneMode)
    {
        SetHeadphoneModeEnabled(bDesiredHeadphoneMode);
    }

    if (DesiredUserChosenDeviceProfileSuffix != UserChosenDeviceProfileSuffix)
    {
        UserChosenDeviceProfileSuffix = DesiredUserChosenDeviceProfileSuffix;
    }

    if (FApp::CanEverRender())
    {
        ApplyDisplayGamma();
        ApplySafeZoneScale();
        UpdateGameModeDeviceProfileAndFps();
    }

    PerfStatSettingsChangedEvent.Broadcast();
}

int32 UUR_SettingsLocal::GetOverallScalabilityLevel() const
{
    int32 Result = Super::GetOverallScalabilityLevel();

    const UUR_PlatformSpecificRenderingSettings* PlatformSettings = UUR_PlatformSpecificRenderingSettings::Get();
    if (PlatformSettings->FramePacingMode == EGameFramePacingMode::MobileStyle)
    {
        Result = GetHighestLevelOfAnyScalabilityChannel();
    }

    return Result;
}

void UUR_SettingsLocal::SetOverallScalabilityLevel(int32 Value)
{
    TGuardValue Guard(bSettingOverallQualityGuard, true);

    Value = FMath::Clamp(Value, 0, 3);

    float CurrentMobileResolutionQuality = ScalabilityQuality.ResolutionQuality;

    Super::SetOverallScalabilityLevel(Value);

    const UUR_PlatformSpecificRenderingSettings* PlatformSettings = UUR_PlatformSpecificRenderingSettings::Get();
    if (PlatformSettings->FramePacingMode == EGameFramePacingMode::MobileStyle)
    {
        // Restore the resolution quality, mobile decouples this from overall quality
        ScalabilityQuality.ResolutionQuality = CurrentMobileResolutionQuality;

        // Changing the overall quality can end up adjusting the frame rate on mobile since there are limits
        const int32 ConstrainedFrameRateLimit = GameSettingsHelpers::ConstrainFrameRateToBeCompatibleWithOverallQuality(DesiredMobileFrameRateLimit, Value);
        if (ConstrainedFrameRateLimit != DesiredMobileFrameRateLimit)
        {
            SetDesiredMobileFrameRateLimit(ConstrainedFrameRateLimit);
        }
    }
}

void UUR_SettingsLocal::SetControllerPlatform(const FName InControllerPlatform)
{
    if (ControllerPlatform != InControllerPlatform)
    {
        ControllerPlatform = InControllerPlatform;

        // Apply the change to the common input subsystem so that we refresh any input icons we're using.
        if (UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetTypedOuter<ULocalPlayer>()))
        {
            InputSubsystem->SetGamepadInputType(ControllerPlatform);
        }
    }
}

FName UUR_SettingsLocal::GetControllerPlatform() const
{
    return ControllerPlatform;
}

void UUR_SettingsLocal::LoadUserControlBusMix()
{
    if (GEngine)
    {
        if (const UWorld* World = GEngine->GetCurrentPlayWorld())
        {
            if (const UUR_AudioSettings* GameAudioSettings = GetDefault<UUR_AudioSettings>())
            {
                USoundControlBus* OverallControlBus = nullptr;
                USoundControlBus* MusicControlBus = nullptr;
                USoundControlBus* SoundFXControlBus = nullptr;
                USoundControlBus* DialogueControlBus = nullptr;
                USoundControlBus* VoiceChatControlBus = nullptr;

                ControlBusMap.Empty();

                if (UObject* ObjPath = GameAudioSettings->OverallVolumeControlBus.TryLoad())
                {
                    if (USoundControlBus* SoundControlBus = Cast<USoundControlBus>(ObjPath))
                    {
                        OverallControlBus = SoundControlBus;
                        ControlBusMap.Add(TEXT("Overall"), OverallControlBus);
                    }
                    else
                    {
                        ensureMsgf(SoundControlBus, TEXT("Overall Control Bus reference missing from Game Audio Settings."));
                    }
                }

                if (UObject* ObjPath = GameAudioSettings->MusicVolumeControlBus.TryLoad())
                {
                    if (USoundControlBus* SoundControlBus = Cast<USoundControlBus>(ObjPath))
                    {
                        MusicControlBus = SoundControlBus;
                        ControlBusMap.Add(TEXT("Music"), MusicControlBus);
                    }
                    else
                    {
                        ensureMsgf(SoundControlBus, TEXT("Music Control Bus reference missing from Game Audio Settings."));
                    }
                }

                if (UObject* ObjPath = GameAudioSettings->SoundFXVolumeControlBus.TryLoad())
                {
                    if (USoundControlBus* SoundControlBus = Cast<USoundControlBus>(ObjPath))
                    {
                        SoundFXControlBus = SoundControlBus;
                        ControlBusMap.Add(TEXT("SoundFX"), SoundFXControlBus);
                    }
                    else
                    {
                        ensureMsgf(SoundControlBus, TEXT("SoundFX Control Bus reference missing from Game Audio Settings."));
                    }
                }

                if (UObject* ObjPath = GameAudioSettings->DialogueVolumeControlBus.TryLoad())
                {
                    if (USoundControlBus* SoundControlBus = Cast<USoundControlBus>(ObjPath))
                    {
                        DialogueControlBus = SoundControlBus;
                        ControlBusMap.Add(TEXT("Dialogue"), DialogueControlBus);
                    }
                    else
                    {
                        ensureMsgf(SoundControlBus, TEXT("Dialogue Control Bus reference missing from Game Audio Settings."));
                    }
                }

                if (UObject* ObjPath = GameAudioSettings->VoiceChatVolumeControlBus.TryLoad())
                {
                    if (USoundControlBus* SoundControlBus = Cast<USoundControlBus>(ObjPath))
                    {
                        VoiceChatControlBus = SoundControlBus;
                        ControlBusMap.Add(TEXT("VoiceChat"), VoiceChatControlBus);
                    }
                    else
                    {
                        ensureMsgf(SoundControlBus, TEXT("VoiceChat Control Bus reference missing from Game Audio Settings."));
                    }
                }

                if (UObject* ObjPath = GameAudioSettings->UserSettingsControlBusMix.TryLoad())
                {
                    if (USoundControlBusMix* SoundControlBusMix = Cast<USoundControlBusMix>(ObjPath))
                    {
                        ControlBusMix = SoundControlBusMix;

                        const FSoundControlBusMixStage OverallControlBusMixStage = UAudioModulationStatics::CreateBusMixStage(World, OverallControlBus, OverallVolume);
                        const FSoundControlBusMixStage MusicControlBusMixStage = UAudioModulationStatics::CreateBusMixStage(World, MusicControlBus, MusicVolume);
                        const FSoundControlBusMixStage SoundFXControlBusMixStage = UAudioModulationStatics::CreateBusMixStage(World, SoundFXControlBus, SoundFXVolume);
                        const FSoundControlBusMixStage DialogueControlBusMixStage = UAudioModulationStatics::CreateBusMixStage(World, DialogueControlBus, DialogueVolume);
                        const FSoundControlBusMixStage VoiceChatControlBusMixStage = UAudioModulationStatics::CreateBusMixStage(World, VoiceChatControlBus, VoiceChatVolume);

                        TArray<FSoundControlBusMixStage> ControlBusMixStageArray;
                        ControlBusMixStageArray.Add(OverallControlBusMixStage);
                        ControlBusMixStageArray.Add(MusicControlBusMixStage);
                        ControlBusMixStageArray.Add(SoundFXControlBusMixStage);
                        ControlBusMixStageArray.Add(DialogueControlBusMixStage);
                        ControlBusMixStageArray.Add(VoiceChatControlBusMixStage);

                        UAudioModulationStatics::UpdateMix(World, ControlBusMix, ControlBusMixStageArray);

                        bSoundControlBusMixLoaded = true;
                    }
                    else
                    {
                        ensureMsgf(SoundControlBusMix, TEXT("User Settings Control Bus Mix reference missing from Game Audio Settings."));
                    }
                }
            }
        }
    }
}

void UUR_SettingsLocal::OnAppActivationStateChanged(bool bIsActive)
{
    // We might want to adjust the frame rate when the app loses/gains focus on multi-window platforms
    UpdateEffectiveFrameRateLimit();
}

void UUR_SettingsLocal::UpdateGameModeDeviceProfileAndFps()
{
#if WITH_EDITOR
    if (GIsEditor && !CVarApplyDeviceProfilesInPIE.GetValueOnGameThread())
    {
        return;
    }
#endif

    UDeviceProfileManager& Manager = UDeviceProfileManager::Get();

    const UUR_PlatformSpecificRenderingSettings* PlatformSettings = UUR_PlatformSpecificRenderingSettings::Get();
    const TArray<FGameQualityDeviceProfileVariant>& UserFacingVariants = PlatformSettings->UserFacingDeviceProfileOptions;

    //@TODO: Might want to allow specific experiences to specify a suffix to attempt to use as well
    // The code below will handle searching with this suffix (alone or in conjunction with the frame rate), but nothing sets it right now
    FString ExperienceSuffix;

    // Make sure the chosen setting is supported for the current display, walking down the list to try fallbacks
    const int32 PlatformMaxRefreshRate = FPlatformMisc::GetMaxRefreshRate();

    int32 SuffixIndex = UserFacingVariants.IndexOfByPredicate
    ([&](const FGameQualityDeviceProfileVariant& Data)
    {
        return Data.DeviceProfileSuffix == UserChosenDeviceProfileSuffix;
    });
    while (UserFacingVariants.IsValidIndex(SuffixIndex))
    {
        if (PlatformMaxRefreshRate >= UserFacingVariants[SuffixIndex].MinRefreshRate)
        {
            break;
        }
        else
        {
            --SuffixIndex;
        }
    }

    const FString EffectiveUserSuffix = UserFacingVariants.IsValidIndex(SuffixIndex) ? UserFacingVariants[SuffixIndex].DeviceProfileSuffix : PlatformSettings->DefaultDeviceProfileSuffix;

    // Build up a list of names to try
    const bool bHadUserSuffix = !EffectiveUserSuffix.IsEmpty();
    const bool bHadExperienceSuffix = !ExperienceSuffix.IsEmpty();

    FString BasePlatformName = UDeviceProfileManager::GetPlatformDeviceProfileName();
    FName PlatformName; // Default unless in editor
#if WITH_EDITOR
    if (GIsEditor)
    {
        const UUR_PlatformEmulationSettings* Settings = GetDefault<UUR_PlatformEmulationSettings>();
        const FName PretendBaseDeviceProfile = Settings->GetPretendBaseDeviceProfile();
        if (PretendBaseDeviceProfile != NAME_None)
        {
            BasePlatformName = PretendBaseDeviceProfile.ToString();
        }

        PlatformName = Settings->GetPretendPlatformName();
    }
#endif

    TArray<FString> ComposedNamesToFind;
    if (bHadExperienceSuffix && bHadUserSuffix)
    {
        ComposedNamesToFind.Add(BasePlatformName + TEXT("_") + ExperienceSuffix + TEXT("_") + EffectiveUserSuffix);
    }
    if (bHadUserSuffix)
    {
        ComposedNamesToFind.Add(BasePlatformName + TEXT("_") + EffectiveUserSuffix);
    }
    if (bHadExperienceSuffix)
    {
        ComposedNamesToFind.Add(BasePlatformName + TEXT("_") + ExperienceSuffix);
    }
    if (GIsEditor)
    {
        ComposedNamesToFind.Add(BasePlatformName);
    }

    // See if any of the potential device profiles actually exists
    FString ActualProfileToApply;
    for (const FString& TestProfileName : ComposedNamesToFind)
    {
        if (Manager.HasLoadableProfileName(TestProfileName, PlatformName))
        {
            ActualProfileToApply = TestProfileName;
            const UDeviceProfile* Profile = Manager.FindProfile(TestProfileName, /*bCreateOnFail=*/ false);
            if (Profile == nullptr)
            {
                Profile = Manager.CreateProfile(TestProfileName, TEXT(""), TestProfileName, *PlatformName.ToString());
            }

            UE_LOG(LogConsoleResponse, Log, TEXT("Profile %s exists"), *Profile->GetName());
            break;
        }
    }

    UE_LOG
    (LogConsoleResponse,
        Log,
        TEXT("UpdateGameModeDeviceProfileAndFps MaxRefreshRate=%d, ExperienceSuffix='%s', UserPicked='%s'->'%s', PlatformBase='%s', AppliedActual='%s'"),
        PlatformMaxRefreshRate,
        *ExperienceSuffix,
        *UserChosenDeviceProfileSuffix,
        *EffectiveUserSuffix,
        *BasePlatformName,
        *ActualProfileToApply);

    // Apply the device profile if it's different to what we currently have
    if (ActualProfileToApply != CurrentAppliedDeviceProfileOverrideSuffix)
    {
        if (Manager.GetActiveDeviceProfileName() != ActualProfileToApply)
        {
            // Restore the default first
            if (GIsEditor)
            {
#if ALLOW_OTHER_PLATFORM_CONFIG
                Manager.RestorePreviewDeviceProfile();
#endif
            }
            else
            {
                Manager.RestoreDefaultDeviceProfile();
            }

            // Apply the new one (if it wasn't the default)
            if (Manager.GetActiveDeviceProfileName() != ActualProfileToApply)
            {
                UDeviceProfile* NewDeviceProfile = Manager.FindProfile(ActualProfileToApply);
                ensureMsgf(NewDeviceProfile != nullptr, TEXT("DeviceProfile %s not found "), *ActualProfileToApply);
                if (NewDeviceProfile)
                {
                    if (GIsEditor)
                    {
#if ALLOW_OTHER_PLATFORM_CONFIG
                        UE_LOG(LogConsoleResponse, Log, TEXT("Overriding *preview* device profile to %s"), *ActualProfileToApply);
                        Manager.SetPreviewDeviceProfile(NewDeviceProfile);

                        // Reload the default settings from the pretend profile
                        GameSettingsHelpers::FillScalabilitySettingsFromDeviceProfile(DeviceDefaultScalabilitySettings);
#endif
                    }
                    else
                    {
                        UE_LOG(LogConsoleResponse, Log, TEXT("Overriding device profile to %s"), *ActualProfileToApply);
                        Manager.SetOverrideDeviceProfile(NewDeviceProfile);
                    }
                }
            }
        }
        CurrentAppliedDeviceProfileOverrideSuffix = ActualProfileToApply;
    }

    switch (PlatformSettings->FramePacingMode)
    {
        case EGameFramePacingMode::MobileStyle:
        {
            UpdateMobileFramePacing();
            break;
        }
        case EGameFramePacingMode::ConsoleStyle:
        {
            UpdateConsoleFramePacing();
            break;
        }
        case EGameFramePacingMode::DesktopStyle:
        {
            UpdateDesktopFramePacing();
            break;
        }
    }
}

void UUR_SettingsLocal::UpdateConsoleFramePacing()
{
    // Apply device-profile-driven frame sync and frame pace
    const int32 FrameSyncType = CVarDeviceProfileDrivenFrameSyncType.GetValueOnGameThread();
    if (FrameSyncType != -1)
    {
        UE_LOG(LogConsoleResponse, Log, TEXT("Setting frame sync mode to %d."), FrameSyncType);
        SetSyncTypeCVar(FrameSyncType);
    }

    const int32 TargetFPS = CVarDeviceProfileDrivenTargetFps.GetValueOnGameThread();
    if (TargetFPS != -1)
    {
        UE_LOG(LogConsoleResponse, Log, TEXT("Setting frame pace to %d Hz."), TargetFPS);
        FPlatformRHIFramePacer::SetFramePace(TargetFPS);

        // Set the CSV metadata and analytics Fps mode strings
#if CSV_PROFILER
        const FString TargetFramerateString = FString::Printf(TEXT("%d"), TargetFPS);
        CSV_METADATA(TEXT("TargetFramerate"), *TargetFramerateString);
#endif
    }
}

void UUR_SettingsLocal::UpdateDesktopFramePacing()
{
    // For desktop the frame rate limit is handled by the parent class based on the value already
    // applied via UpdateEffectiveFrameRateLimit()
    // So this function is only doing 'second order' effects of desktop frame pacing preferences

    const float TargetFPS = GetEffectiveFrameRateLimit();
    const float ClampedFPS = (TargetFPS <= 0.0f) ? 60.0f : FMath::Clamp(TargetFPS, 30.0f, 60.0f);
    UpdateDynamicResFrameTime(ClampedFPS);
}

void UUR_SettingsLocal::UpdateMobileFramePacing()
{
    //@TODO: Handle different limits for in-front-end or low-battery mode on mobile

    // Choose the closest supported frame rate to the user desired setting without going over the device imposed limit
    const UUR_PlatformSpecificRenderingSettings* PlatformSettings = UUR_PlatformSpecificRenderingSettings::Get();
    const TArray<int32>& PossibleRates = PlatformSettings->MobileFrameRateLimits;
    const int32 LimitIndex = PossibleRates.FindLastByPredicate
    ([this](const int32& TestRate)
    {
        return (TestRate <= MobileFrameRateLimit) && IsSupportedMobileFramePace(TestRate);
    });
    const int32 TargetFPS = PossibleRates.IsValidIndex(LimitIndex) ? PossibleRates[LimitIndex] : GetDefaultMobileFrameRate();

    UE_LOG(LogConsoleResponse, Log, TEXT("Setting frame pace to %d Hz."), TargetFPS);
    FPlatformRHIFramePacer::SetFramePace(TargetFPS);

    ClampMobileQuality();

    UpdateDynamicResFrameTime(static_cast<float>(TargetFPS));
}

void UUR_SettingsLocal::UpdateDynamicResFrameTime(const float TargetFPS)
{
    static IConsoleVariable* CVarDyResFrameTimeBudget = IConsoleManager::Get().FindConsoleVariable(TEXT("r.DynamicRes.FrameTimeBudget"));
    if (CVarDyResFrameTimeBudget)
    {
        if (ensure(TargetFPS > 0.0f))
        {
            const float DyResFrameTimeBudget = 1000.0f / TargetFPS;
            CVarDyResFrameTimeBudget->Set(DyResFrameTimeBudget, ECVF_SetByGameSetting);
        }
    }
}
