// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameSettingValueDiscrete.h"

#include "UR_SettingValueDiscrete_OverallQuality.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UObject;

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class UUR_SettingValueDiscrete_OverallQuality : public UGameSettingValueDiscrete
{
    GENERATED_BODY()

public:
    UUR_SettingValueDiscrete_OverallQuality();

    /** UGameSettingValue */
    virtual void StoreInitial() override;
    virtual void ResetToDefault() override;
    virtual void RestoreToInitial() override;

    /** UGameSettingValueDiscrete */
    virtual void SetDiscreteOptionByIndex(int32 Index) override;
    virtual int32 GetDiscreteOptionIndex() const override;
    virtual TArray<FText> GetDiscreteOptions() const override;

protected:
    /** UGameSettingValue */
    virtual void OnInitialized() override;

    int32 GetCustomOptionIndex() const;
    int32 GetOverallQualityLevel() const;

    TArray<FText> Options;
    TArray<FText> OptionsWithCustom;
};
