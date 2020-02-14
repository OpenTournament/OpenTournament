// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_WidgetMatchTimer.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_WidgetMatchTimer::UUR_WidgetMatchTimer(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer),
    RemainingTime(-1)
{

}

FString UUR_WidgetMatchTimer::GetTimeString(const float TimeSeconds)
{
    // only minutes and seconds are relevant
    const int32 TotalSeconds = FMath::Max(0, FMath::TruncToInt(TimeSeconds) % 3600);
    const int32 NumMinutes = TotalSeconds / 60;
    const int32 NumSeconds = TotalSeconds % 60;

    const FString TimeDesc = FString::Printf(TEXT("%02d:%02d"), NumMinutes, NumSeconds);
    return TimeDesc;
}