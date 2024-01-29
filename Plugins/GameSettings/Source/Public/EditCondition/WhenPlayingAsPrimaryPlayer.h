// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameSettingFilterState.h"

class ULocalPlayer;


class GAMESETTINGS_API FWhenPlayingAsPrimaryPlayer : public FGameSettingEditCondition
{
public:
	static TSharedRef<FWhenPlayingAsPrimaryPlayer> Get();

	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer, FGameSettingEditableState& InOutEditState) const override;
};
