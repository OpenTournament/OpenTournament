// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_SettingValueDiscrete_PerfStat.h"

#include "CommonUIVisibilitySubsystem.h"

#include "Performance/UR_PerformanceSettings.h"
#include "Performance/UR_PerformanceStatTypes.h"
#include "Settings/UR_SettingsLocal.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_SettingValueDiscrete_PerfStat)

/////////////////////////////////////////////////////////////////////////////////////////////////

class ULocalPlayer;

/////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "OTSettings"

//////////////////////////////////////////////////////////////////////

class FGameSettingEditCondition_PerfStatAllowed : public FGameSettingEditCondition
{
public:
    static TSharedRef<FGameSettingEditCondition_PerfStatAllowed> Get(EGameDisplayablePerformanceStat Stat)
    {
        return MakeShared<FGameSettingEditCondition_PerfStatAllowed>(Stat);
    }

    FGameSettingEditCondition_PerfStatAllowed(EGameDisplayablePerformanceStat Stat)
        : AssociatedStat(Stat)
    {}

    //~FGameSettingEditCondition interface
    virtual void GatherEditState(const ULocalPlayer* InLocalPlayer, FGameSettingEditableState& InOutEditState) const override
    {
        const FGameplayTagContainer& VisibilityTags = UCommonUIVisibilitySubsystem::GetChecked(InLocalPlayer)->GetVisibilityTags();

        bool bCanShowStat = false;
        for (const FGamePerformanceStatGroup& Group : GetDefault<UUR_PerformanceSettings>()->UserFacingPerformanceStats) //@TODO: Move this stuff to per-platform instead of doing vis queries too?
        {
            if (Group.AllowedStats.Contains(AssociatedStat))
            {
                const bool bShowGroup = (Group.VisibilityQuery.IsEmpty() || Group.VisibilityQuery.Matches(VisibilityTags));
                if (bShowGroup)
                {
                    bCanShowStat = true;
                    break;
                }
            }
        }

        if (!bCanShowStat)
        {
            InOutEditState.Hide(TEXT("Stat is not listed in UUR_PerformanceSettings or is suppressed by current platform traits"));
        }
    }

    //~End of FGameSettingEditCondition interface

private:
    EGameDisplayablePerformanceStat AssociatedStat;
};

//////////////////////////////////////////////////////////////////////

UUR_SettingValueDiscrete_PerfStat::UUR_SettingValueDiscrete_PerfStat()
{}

void UUR_SettingValueDiscrete_PerfStat::SetStat(EGameDisplayablePerformanceStat InStat)
{
    StatToDisplay = InStat;
    SetDevName(FName(*FString::Printf(TEXT("PerfStat_%d"), (int32)StatToDisplay)));
    AddEditCondition(FGameSettingEditCondition_PerfStatAllowed::Get(StatToDisplay));
}

void UUR_SettingValueDiscrete_PerfStat::AddMode(FText&& Label, EGameStatDisplayMode Mode)
{
    Options.Emplace(MoveTemp(Label));
    DisplayModes.Add(Mode);
}

void UUR_SettingValueDiscrete_PerfStat::OnInitialized()
{
    Super::OnInitialized();

    AddMode(LOCTEXT("PerfStatDisplayMode_None", "None"), EGameStatDisplayMode::Hidden);
    AddMode(LOCTEXT("PerfStatDisplayMode_TextOnly", "Text Only"), EGameStatDisplayMode::TextOnly);
    AddMode(LOCTEXT("PerfStatDisplayMode_GraphOnly", "Graph Only"), EGameStatDisplayMode::GraphOnly);
    AddMode(LOCTEXT("PerfStatDisplayMode_TextAndGraph", "Text and Graph"), EGameStatDisplayMode::TextAndGraph);
}

void UUR_SettingValueDiscrete_PerfStat::StoreInitial()
{
    const UUR_SettingsLocal* Settings = UUR_SettingsLocal::Get();
    InitialMode = Settings->GetPerfStatDisplayState(StatToDisplay);
}

void UUR_SettingValueDiscrete_PerfStat::ResetToDefault()
{
    UUR_SettingsLocal* Settings = UUR_SettingsLocal::Get();
    Settings->SetPerfStatDisplayState(StatToDisplay, EGameStatDisplayMode::Hidden);
    NotifySettingChanged(EGameSettingChangeReason::ResetToDefault);
}

void UUR_SettingValueDiscrete_PerfStat::RestoreToInitial()
{
    UUR_SettingsLocal* Settings = UUR_SettingsLocal::Get();
    Settings->SetPerfStatDisplayState(StatToDisplay, InitialMode);
    NotifySettingChanged(EGameSettingChangeReason::RestoreToInitial);
}

void UUR_SettingValueDiscrete_PerfStat::SetDiscreteOptionByIndex(int32 Index)
{
    if (DisplayModes.IsValidIndex(Index))
    {
        const EGameStatDisplayMode DisplayMode = DisplayModes[Index];

        UUR_SettingsLocal* Settings = UUR_SettingsLocal::Get();
        Settings->SetPerfStatDisplayState(StatToDisplay, DisplayMode);
    }
    NotifySettingChanged(EGameSettingChangeReason::Change);
}

int32 UUR_SettingValueDiscrete_PerfStat::GetDiscreteOptionIndex() const
{
    const UUR_SettingsLocal* Settings = UUR_SettingsLocal::Get();
    return DisplayModes.Find(Settings->GetPerfStatDisplayState(StatToDisplay));
}

TArray<FText> UUR_SettingValueDiscrete_PerfStat::GetDiscreteOptions() const
{
    return Options;
}

#undef LOCTEXT_NAMESPACE
