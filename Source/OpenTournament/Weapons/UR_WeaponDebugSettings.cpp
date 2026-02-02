// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_WeaponDebugSettings.h"

#include "Misc/App.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_WeaponDebugSettings)

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_WeaponDebugSettings::UUR_WeaponDebugSettings()
    : DrawBulletTraceDuration(0.0)
    , DrawBulletHitDuration(0.0)
    , DrawBulletHitRadius(0.0)
{}

FName UUR_WeaponDebugSettings::GetCategoryName() const
{
    return FApp::GetProjectName();
}
