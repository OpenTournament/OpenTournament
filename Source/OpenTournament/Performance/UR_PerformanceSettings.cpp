// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_PerformanceSettings.h"

#include "Engine/PlatformSettingsManager.h"
#include "Misc/EnumRange.h"
#include "Performance/UR_PerformanceStatTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_PerformanceSettings)

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_PlatformSpecificRenderingSettings::UUR_PlatformSpecificRenderingSettings()
{
    MobileFrameRateLimits.Append({ 20, 30, 45, 60, 90, 120 });
}

const UUR_PlatformSpecificRenderingSettings* UUR_PlatformSpecificRenderingSettings::Get()
{
    UUR_PlatformSpecificRenderingSettings* Result = UPlatformSettingsManager::Get().GetSettingsForPlatform<ThisClass>();
    check(Result);
    return Result;
}

//////////////////////////////////////////////////////////////////////

UUR_PerformanceSettings::UUR_PerformanceSettings()
{
    PerPlatformSettings.Initialize(UUR_PlatformSpecificRenderingSettings::StaticClass());

    CategoryName = TEXT("Game");

    DesktopFrameRateLimits.Append({ 30, 60, 120, 144, 160, 165, 180, 200, 240, 360 });

    // Default to all stats are allowed
    FGamePerformanceStatGroup& StatGroup = UserFacingPerformanceStats.AddDefaulted_GetRef();
    for (EGameDisplayablePerformanceStat PerfStat : TEnumRange<EGameDisplayablePerformanceStat>())
    {
        StatGroup.AllowedStats.Add(PerfStat);
    }
}
