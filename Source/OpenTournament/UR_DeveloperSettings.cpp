// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_DeveloperSettings.h"
#include "Framework/Notifications/NotificationManager.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Misc/App.h"
#include "Widgets/Notifications/SNotificationList.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_DeveloperSettings)

/////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "UR_Cheats"

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_DeveloperSettings::UUR_DeveloperSettings()
{
}

FName UUR_DeveloperSettings::GetCategoryName() const
{
    return FApp::GetProjectName();
}

#if WITH_EDITOR
void UUR_DeveloperSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    ApplySettings();
}

void UUR_DeveloperSettings::PostReloadConfig(FProperty* PropertyThatWasLoaded)
{
    Super::PostReloadConfig(PropertyThatWasLoaded);

    ApplySettings();
}

void UUR_DeveloperSettings::PostInitProperties()
{
    Super::PostInitProperties();

    ApplySettings();
}

void UUR_DeveloperSettings::ApplySettings()
{
}

void UUR_DeveloperSettings::OnPlayInEditorStarted() const
{
    // Show a notification toast to remind the user that there's an experience override set
    if (ExperienceOverride.IsValid())
    {
        FNotificationInfo Info(FText::Format(
            LOCTEXT("ExperienceOverrideActive", "Developer Settings Override\nExperience {0}"),
            FText::FromName(ExperienceOverride.PrimaryAssetName)
        ));
        Info.ExpireDuration = 2.0f;
        FSlateNotificationManager::Get().AddNotification(Info);
    }
}
#endif

#undef LOCTEXT_NAMESPACE
