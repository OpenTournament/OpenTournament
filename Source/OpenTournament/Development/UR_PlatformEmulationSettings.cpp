// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_PlatformEmulationSettings.h"

#include "CommonUIVisibilitySubsystem.h"
#include "Engine/PlatformSettingsManager.h"
#include "Misc/App.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "DeviceProfiles/DeviceProfileManager.h"
#include "DeviceProfiles/DeviceProfile.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_PlatformEmulationSettings)

/////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "GameCheats"

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_PlatformEmulationSettings::UUR_PlatformEmulationSettings()
{}

FName UUR_PlatformEmulationSettings::GetCategoryName() const
{
    return FApp::GetProjectName();
}

FName UUR_PlatformEmulationSettings::GetPretendBaseDeviceProfile() const
{
    return PretendBaseDeviceProfile;
}

FName UUR_PlatformEmulationSettings::GetPretendPlatformName() const
{
    return PretendPlatform;
}

#if WITH_EDITOR
void UUR_PlatformEmulationSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    ApplySettings();
}

void UUR_PlatformEmulationSettings::PostReloadConfig(FProperty* PropertyThatWasLoaded)
{
    Super::PostReloadConfig(PropertyThatWasLoaded);

    ApplySettings();
}

void UUR_PlatformEmulationSettings::PostInitProperties()
{
    Super::PostInitProperties();

    ApplySettings();
}

void UUR_PlatformEmulationSettings::OnPlayInEditorStarted() const
{
    // Show a notification toast to remind the user that there's a tag enable override set
    if (!AdditionalPlatformTraitsToEnable.IsEmpty())
    {
        FNotificationInfo Info
        (FText::Format
            (
                LOCTEXT("PlatformTraitEnableActive", "Platform Trait Override\nEnabling {0}"),
                FText::AsCultureInvariant(AdditionalPlatformTraitsToEnable.ToStringSimple())
            ));
        Info.ExpireDuration = 3.0f;
        FSlateNotificationManager::Get().AddNotification(Info);
    }

    // Show a notification toast to remind the user that there's a tag suppression override set
    if (!AdditionalPlatformTraitsToSuppress.IsEmpty())
    {
        FNotificationInfo Info
        (FText::Format
            (
                LOCTEXT("PlatformTraitSuppressionActive", "Platform Trait Override\nSuppressing {0}"),
                FText::AsCultureInvariant(AdditionalPlatformTraitsToSuppress.ToStringSimple())
            ));
        Info.ExpireDuration = 3.0f;
        FSlateNotificationManager::Get().AddNotification(Info);
    }

    // Show a notification toast to remind the user that there's a platform override set
    if (PretendPlatform != NAME_None)
    {
        FNotificationInfo Info
        (FText::Format
            (
                LOCTEXT("PlatformOverrideActive", "Platform Override Active\nPretending to be {0}"),
                FText::FromName(PretendPlatform)
            ));
        Info.ExpireDuration = 3.0f;
        FSlateNotificationManager::Get().AddNotification(Info);
    }
}

void UUR_PlatformEmulationSettings::ApplySettings()
{
    UCommonUIVisibilitySubsystem::SetDebugVisibilityConditions(AdditionalPlatformTraitsToEnable, AdditionalPlatformTraitsToSuppress);

    if (GIsEditor && PretendPlatform != LastAppliedPretendPlatform)
    {
        ChangeActivePretendPlatform(PretendPlatform);
    }

    PickReasonableBaseDeviceProfile();
}

void UUR_PlatformEmulationSettings::ChangeActivePretendPlatform(FName NewPlatformName)
{
    LastAppliedPretendPlatform = NewPlatformName;
    PretendPlatform = NewPlatformName;

    UPlatformSettingsManager::SetEditorSimulatedPlatform(PretendPlatform);
}

#endif

TArray<FName> UUR_PlatformEmulationSettings::GetKnownPlatformIds() const
{
    TArray<FName> Results;

#if WITH_EDITOR
    Results.Add(NAME_None);
    Results.Append(UPlatformSettingsManager::GetKnownAndEnablePlatformIniNames());
#endif

    return Results;
}

TArray<FName> UUR_PlatformEmulationSettings::GetKnownDeviceProfiles() const
{
    TArray<FName> Results;

#if WITH_EDITOR
    const UDeviceProfileManager& Manager = UDeviceProfileManager::Get();
    Results.Reserve(Manager.Profiles.Num() + 1);

    if (PretendPlatform == NAME_None)
    {
        Results.Add(NAME_None);
    }

    for (const TObjectPtr<UDeviceProfile>& Profile : Manager.Profiles)
    {
        bool bIncludeEntry = true;
        if (PretendPlatform != NAME_None)
        {
            if (Profile->DeviceType != PretendPlatform.ToString())
            {
                bIncludeEntry = false;
            }
        }

        if (bIncludeEntry)
        {
            Results.Add(Profile->GetFName());
        }
    }
#endif

    return Results;
}

void UUR_PlatformEmulationSettings::PickReasonableBaseDeviceProfile()
{
    // First see if our pretend device profile is already compatible, if so we don't need to do anything
    UDeviceProfileManager& Manager = UDeviceProfileManager::Get();
    if (UDeviceProfile* ProfilePtr = Manager.FindProfile(PretendBaseDeviceProfile.ToString(), /*bCreateOnFail=*/ false))
    {
        const bool bIsCompatible = (PretendPlatform == NAME_None) || (ProfilePtr->DeviceType == PretendPlatform.ToString());
        if (!bIsCompatible)
        {
            PretendBaseDeviceProfile = NAME_None;
        }
    }

    if ((PretendPlatform != NAME_None) && (PretendBaseDeviceProfile == NAME_None))
    {
        // If we're pretending we're a platform and don't have a pretend base profile, pick a reasonable one,
        // preferring the one with the shortest name as a simple heuristic
        FName ShortestMatchingProfileName;
        const FString PretendPlatformStr = PretendPlatform.ToString();
        for (const TObjectPtr<UDeviceProfile>& Profile : Manager.Profiles)
        {
            if (Profile->DeviceType == PretendPlatformStr)
            {
                const FName TestName = Profile->GetFName();
                if ((ShortestMatchingProfileName == NAME_None) || (TestName.GetStringLength() < ShortestMatchingProfileName.GetStringLength()))
                {
                    ShortestMatchingProfileName = TestName;
                }
            }
        }
        PretendBaseDeviceProfile = ShortestMatchingProfileName;
    }
}

#undef LOCTEXT_NAMESPACE
