// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameSettingFilterState.h"
#include "GameplayTagContainer.h"

class ULocalPlayer;

//////////////////////////////////////////////////////////////////////
// FWhenPlatformHasTrait

// Edit condition for game settings that checks CommonUI's platform traits
// to determine whether or not to show a setting
class GAMESETTINGS_API FWhenPlatformHasTrait : public FGameSettingEditCondition
{
public:
	static TSharedRef<FWhenPlatformHasTrait> KillIfMissing(FGameplayTag InVisibilityTag, const FString& InKillReason);
	static TSharedRef<FWhenPlatformHasTrait> DisableIfMissing(FGameplayTag InVisibilityTag, const FText& InDisableReason);

	static TSharedRef<FWhenPlatformHasTrait> KillIfPresent(FGameplayTag InVisibilityTag, const FString& InKillReason);
	static TSharedRef<FWhenPlatformHasTrait> DisableIfPresent(FGameplayTag InVisibilityTag, const FText& InDisableReason);

	//~FGameSettingEditCondition interface
	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer, FGameSettingEditableState& InOutEditState) const override;
	//~End of FGameSettingEditCondition interface

private:
	FGameplayTag VisibilityTag;
	bool bTagDesired;
	FString KillReason;
	FText DisableReason;
};
