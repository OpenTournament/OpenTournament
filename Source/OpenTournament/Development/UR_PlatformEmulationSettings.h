// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Engine/DeveloperSettingsBackedByCVars.h"

#include "GameplayTagContainer.h"

#include "UR_PlatformEmulationSettings.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

#define UE_API OPENTOURNAMENT_API

struct FPropertyChangedEvent;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Platform emulation settings
 */
UCLASS(MinimalAPI, Config=EditorPerProjectUserSettings, DisplayName = "Platform Emulation Settings")
class UUR_PlatformEmulationSettings : public UDeveloperSettingsBackedByCVars
{
    GENERATED_BODY()

public:
    UUR_PlatformEmulationSettings();

    //~UDeveloperSettings interface
    virtual FName GetCategoryName() const override;
    //~End of UDeveloperSettings interface

    FName GetPretendBaseDeviceProfile() const;
    FName GetPretendPlatformName() const;

private:
    UPROPERTY(EditAnywhere, config, Category=PlatformEmulation, meta=(Categories="Input,Platform.Trait"))
    FGameplayTagContainer AdditionalPlatformTraitsToEnable;

    UPROPERTY(EditAnywhere, config, Category=PlatformEmulation, meta=(Categories="Input,Platform.Trait"))
    FGameplayTagContainer AdditionalPlatformTraitsToSuppress;

    UPROPERTY(EditAnywhere, config, Category=PlatformEmulation, meta=(GetOptions=GetKnownPlatformIds))
    FName PretendPlatform;

    // The base device profile to pretend we are using when emulating device-specific device profiles applied from UUR_SettingsLocal
    UPROPERTY(EditAnywhere, config, Category=PlatformEmulation, meta=(GetOptions=GetKnownDeviceProfiles, EditCondition=bApplyDeviceProfilesInPIE))
    FName PretendBaseDeviceProfile;

    // Do we apply desktop-style frame rate settings in PIE?
    // (frame rate limits are an engine-wide setting so it's not always desirable to have enabled in the editor)
    // You may also want to disable the editor preference "Use Less CPU when in Background" if testing background frame rate limits
    UPROPERTY(EditAnywhere, config, Category=PlatformEmulation, meta=(ConsoleVariable="OT.Settings.ApplyFrameRateSettingsInPIE"))
    bool bApplyFrameRateSettingsInPIE = false;

    // Do we apply front-end specific performance options in PIE?
    // Most engine performance/scalability settings they drive are global, so if one PIE window
    // is in the front-end and the other is in-game one will win and the other gets stuck with those settings
    UPROPERTY(EditAnywhere, config, Category=PlatformEmulation, meta=(ConsoleVariable="OT.Settings.ApplyFrontEndPerformanceOptionsInPIE"))
    bool bApplyFrontEndPerformanceOptionsInPIE = false;

    // Should we apply experience/platform emulated device profiles in PIE?
    UPROPERTY(EditAnywhere, config, Category=PlatformEmulation, meta=(InlineEditConditionToggle, ConsoleVariable="OT.Settings.ApplyDeviceProfilesInPIE"))
    bool bApplyDeviceProfilesInPIE = false;

#if WITH_EDITOR

public:
    // Called by the editor engine to let us pop reminder notifications when cheats are active
    UE_API void OnPlayInEditorStarted() const;

private:
    // The last pretend platform we applied
    FName LastAppliedPretendPlatform;

private:
    void ApplySettings();
    void ChangeActivePretendPlatform(FName NewPlatformName);
#endif

public:
    //~UObject interface
#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    virtual void PostReloadConfig(FProperty* PropertyThatWasLoaded) override;
    virtual void PostInitProperties() override;
#endif
    //~End of UObject interface

private:
    UFUNCTION()
    TArray<FName> GetKnownPlatformIds() const;

    UFUNCTION()
    TArray<FName> GetKnownDeviceProfiles() const;

    void PickReasonableBaseDeviceProfile();
};

#undef UE_API
