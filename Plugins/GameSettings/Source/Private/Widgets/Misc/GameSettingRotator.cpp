// Copyright Epic Games, Inc. All Rights Reserved.

#include "Widgets/Misc/GameSettingRotator.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameSettingRotator)

UGameSettingRotator::UGameSettingRotator(const FObjectInitializer& Initializer)
	: Super(Initializer)
{
}

void UGameSettingRotator::SetDefaultOption(int32 DefaultOptionIndex)
{
	BP_OnDefaultOptionSpecified(DefaultOptionIndex);
}

