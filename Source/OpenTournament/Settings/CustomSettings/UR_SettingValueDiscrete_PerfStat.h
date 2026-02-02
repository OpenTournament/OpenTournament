// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameSettingValueDiscrete.h"

#include "UR_SettingValueDiscrete_PerfStat.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

enum class EGameDisplayablePerformanceStat : uint8;
enum class EGameStatDisplayMode : uint8;

class UObject;

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class UUR_SettingValueDiscrete_PerfStat : public UGameSettingValueDiscrete
{
    GENERATED_BODY()

public:
    UUR_SettingValueDiscrete_PerfStat();

    void SetStat(EGameDisplayablePerformanceStat InStat);

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

    void AddMode(FText&& Label, EGameStatDisplayMode Mode);

protected:
    TArray<FText> Options;
    TArray<EGameStatDisplayMode> DisplayModes;

    EGameDisplayablePerformanceStat StatToDisplay;
    EGameStatDisplayMode InitialMode;
};
