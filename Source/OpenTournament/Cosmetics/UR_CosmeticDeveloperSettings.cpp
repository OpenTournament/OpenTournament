// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_CosmeticDeveloperSettings.h"

#include "Cosmetics/UR_CharacterPartTypes.h"
#include "Misc/App.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
//#include "System/UR_DevelopmentStatics.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
//#include "UR_ControllerComponent_CharacterParts.h"
#include "EngineUtils.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_CosmeticDeveloperSettings)

/////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "GameCheats"

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_CosmeticDeveloperSettings::UUR_CosmeticDeveloperSettings()
    : CheatMode(ECosmeticCheatMode::AddParts)
{}

FName UUR_CosmeticDeveloperSettings::GetCategoryName() const
{
    return FApp::GetProjectName();
}

#if WITH_EDITOR

void UUR_CosmeticDeveloperSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    ApplySettings();
}

void UUR_CosmeticDeveloperSettings::PostReloadConfig(FProperty* PropertyThatWasLoaded)
{
    Super::PostReloadConfig(PropertyThatWasLoaded);

    ApplySettings();
}

void UUR_CosmeticDeveloperSettings::PostInitProperties()
{
    Super::PostInitProperties();

    ApplySettings();
}

void UUR_CosmeticDeveloperSettings::ApplySettings()
{
    if (GIsEditor && (GEngine != nullptr))
    {
        ReapplyLoadoutIfInPIE();
    }
}

void UUR_CosmeticDeveloperSettings::ReapplyLoadoutIfInPIE()
{
#if WITH_SERVER_CODE
    // Update the loadout on all players
    // UWorld* ServerWorld = UUR_DevelopmentStatics::FindPlayInEditorAuthorityWorld();
    // if (ServerWorld != nullptr)
    // {
    // 	ServerWorld->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([=]()
    // 		{
    // 			for (TActorIterator<APlayerController> PCIterator(ServerWorld); PCIterator; ++PCIterator)
    // 			{
    // 				if (APlayerController* PC = *PCIterator)
    // 				{
    // 					if (UUR_ControllerComponent_CharacterParts* CosmeticComponent = PC->FindComponentByClass<UUR_ControllerComponent_CharacterParts>())
    // 					{
    // 						CosmeticComponent->ApplyDeveloperSettings();
    // 					}
    // 				}
    // 			}
    // 		}));
    // }
#endif	// WITH_SERVER_CODE
}

void UUR_CosmeticDeveloperSettings::OnPlayInEditorStarted() const
{
    // Show a notification toast to remind the user that there's an experience override set
    if (CheatCosmeticCharacterParts.Num() > 0)
    {
        FNotificationInfo Info(LOCTEXT("CosmeticOverrideActive", "Applying Cosmetic Override"));
        Info.ExpireDuration = 2.0f;
        FSlateNotificationManager::Get().AddNotification(Info);
    }
}

#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE
