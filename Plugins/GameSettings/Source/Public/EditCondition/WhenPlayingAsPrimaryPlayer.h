// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameSettingFilterState.h"

#define UE_API GAMESETTINGS_API

class ULocalPlayer;


class FWhenPlayingAsPrimaryPlayer : public FGameSettingEditCondition
{
public:
	static UE_API TSharedRef<FWhenPlayingAsPrimaryPlayer> Get();

	UE_API virtual void GatherEditState(const ULocalPlayer* InLocalPlayer, FGameSettingEditableState& InOutEditState) const override;
};

#undef UE_API
