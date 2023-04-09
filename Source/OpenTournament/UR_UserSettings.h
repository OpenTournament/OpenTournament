// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"

#include "UR_Type_WeaponGroup.h"
#include "UR_CharacterCustomization.h"

#include "UR_UserSettings.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AUR_PlayerController;

/////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * 
 */
UCLASS(Config = OTUserSettings, ConfigDoNotCheckDefaults, BlueprintType)
class OPENTOURNAMENT_API UUR_UserSettings : public UObject
{
    GENERATED_BODY()

public:

    UPROPERTY(Config, BlueprintReadWrite)
    int32 NormalFOV;

    UPROPERTY(Config, BlueprintReadWrite)
    FLinearColor AllyColor;

    UPROPERTY(Config, BlueprintReadWrite)
    FLinearColor EnemyColor;

    UPROPERTY(Config, BlueprintReadWrite)
    FLinearColor EnemyColor2;

    UPROPERTY(Config, BlueprintReadWrite)
    FLinearColor EnemyColor3;

    UPROPERTY(Config, BlueprintReadWrite)
    TArray<FWeaponGroup> WeaponGroups;

    UPROPERTY(Config, BlueprintReadWrite)
    FCharacterCustomization CharacterCustomization;

public:

    /**
    * Get UserSettings for PlayerController at index.
    */
    UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "Settings", Meta = (DisplayName = "Get UR UserSettings", WorldContext = "WorldContext"))
    static UUR_UserSettings* Get(const UObject* WorldContext, int32 PlayerIndex = 0);

    /**
    * Create a new UserSettings object, loading in current cached config.
    * Does not replace user's current settings, any modification to the returned object will have no effect.
    * Calling SaveConfig on the returned object will overwrite ini, causing changes to appear upon next load.
    */
    UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Settings", Meta = (DisplayName = "Make UR UserSettings"))
    static UUR_UserSettings* Create()
    {
        return NewObject<UUR_UserSettings>();
    }

    UFUNCTION(BlueprintCallable, Meta = (DisplayName = "SaveConfig"))
    virtual void K2_SaveConfig()
    {
        SaveConfig();
    }

};
