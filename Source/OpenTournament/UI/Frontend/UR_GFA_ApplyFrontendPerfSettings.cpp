// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_GFA_ApplyFrontendPerfSettings.h"

#include "Settings/UR_SettingsLocal.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_GFA_ApplyFrontendPerfSettings)

/////////////////////////////////////////////////////////////////////////////////////////////////

struct FGameFeatureActivatingContext;
struct FGameFeatureDeactivatingContext;

//////////////////////////////////////////////////////////////////////
// UUR_GFA_ApplyFrontendPerfSettings

// Game user settings (and engine performance/scalability settings they drive)
// are global, so there's no point in tracking this per world for multi-player PIE:
// we just apply it if any PIE world is in the menu.
//
// However, by default we won't apply front-end performance stuff in the editor
// unless the developer setting ApplyFrontEndPerformanceOptionsInPIE is enabled
int32 UUR_GFA_ApplyFrontendPerfSettings::ApplicationCounter = 0;

void UUR_GFA_ApplyFrontendPerfSettings::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
    ApplicationCounter++;
    if (ApplicationCounter == 1)
    {
        UUR_SettingsLocal::Get()->SetShouldUseFrontendPerformanceSettings(true);
    }
}

void UUR_GFA_ApplyFrontendPerfSettings::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
    ApplicationCounter--;
    check(ApplicationCounter >= 0);

    if (ApplicationCounter == 0)
    {
        UUR_SettingsLocal::Get()->SetShouldUseFrontendPerformanceSettings(false);
    }
}
