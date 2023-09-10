// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameSettingFilterState.h"

class FWhenCondition : public FGameSettingEditCondition
{
public:
	FWhenCondition(TFunction<void(const ULocalPlayer* InLocalPlayer, FGameSettingEditableState&)>&& InInlineEditCondition)
		: InlineEditCondition(InInlineEditCondition)
	{
	}

	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer, FGameSettingEditableState& InOutEditState) const override
	{
		InlineEditCondition(InLocalPlayer, InOutEditState);
	}

	virtual FString ToString() const override
	{
		return TEXT("Inline Edit Condition");
	}

private:
	TFunction<void(const ULocalPlayer* InLocalPlayer, FGameSettingEditableState& InOutEditState)> InlineEditCondition;
};
