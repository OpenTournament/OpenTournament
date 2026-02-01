// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Engine/DeveloperSettingsBackedByCVars.h"

#include "UR_WeaponDebugCVars.h"

#include "UR_WeaponDebugSettings.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UObject;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Developer debugging settings for weapons
 */
UCLASS(Config=EditorPerProjectUserSettings, DisplayName="Weapon Debug Settings")
class UUR_WeaponDebugSettings : public UDeveloperSettingsBackedByCVars
{
    GENERATED_BODY()

public:
    UUR_WeaponDebugSettings();

    //~UDeveloperSettings interface
    virtual FName GetCategoryName() const override;
    //~End of UDeveloperSettings interface

public:
    // Should we do debug drawing for bullet traces (if above zero, sets how long (in seconds)
    UPROPERTY(config, EditAnywhere, Category=General, meta=(ConsoleVariable="OT.Weapon.DrawBulletTraceDuration", ForceUnits=s))
    float DrawBulletTraceDuration;

    // Should we do debug drawing for bullet impacts (if above zero, sets how long (in seconds)
    UPROPERTY(config, EditAnywhere, Category = General, meta = (ConsoleVariable = "OT.Weapon.DrawBulletHitDuration", ForceUnits = s))
    float DrawBulletHitDuration;

    // When bullet hit debug drawing is enabled (see DrawBulletHitDuration), how big should the hit radius be? (in cm)
    UPROPERTY(config, EditAnywhere, Category = General, meta = (ConsoleVariable = "OT.Weapon.DrawBulletHitRadius", ForceUnits=cm))
    float DrawBulletHitRadius;
};
