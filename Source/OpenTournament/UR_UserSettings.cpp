// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_UserSettings.h"

#include "Kismet/GameplayStatics.h"

#include "UR_BasePlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_UserSettings)

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_UserSettings* UUR_UserSettings::Get(const UObject* WorldContext, int32 PlayerIndex)
{
	if (const auto PC = Cast<AUR_BasePlayerController>(WorldContext))
	{
		return PC->GetUserSettings();
	}
	if (const auto PC = Cast<AUR_BasePlayerController>(UGameplayStatics::GetPlayerController(WorldContext, PlayerIndex)))
	{
		return PC->GetUserSettings();
	}
	return nullptr;
}
