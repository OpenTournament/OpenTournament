// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameSettingAction.h"
#include "GameSettingValueScalarDynamic.h"

#include "UR_SettingAction_SafeZoneEditor.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UGameSetting;
class UObject;

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class UUR_SettingValueScalarDynamic_SafeZoneValue : public UGameSettingValueScalarDynamic
{
    GENERATED_BODY()

public:
    virtual void ResetToDefault() override;
    virtual void RestoreToInitial() override;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class UUR_SettingAction_SafeZoneEditor : public UGameSettingAction
{
    GENERATED_BODY()

public:
    UUR_SettingAction_SafeZoneEditor();
    virtual TArray<UGameSetting*> GetChildSettings() override;

private:
    UPROPERTY()
    TObjectPtr<UUR_SettingValueScalarDynamic_SafeZoneValue> SafeZoneValueSetting;
};
