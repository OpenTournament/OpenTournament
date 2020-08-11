// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_UserSettings.h"

#include "Kismet/GameplayStatics.h"
#include "UR_PlayerController.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_UserSettings* UUR_UserSettings::Get(const UObject* WorldContext, int32 PlayerIndex)
{
	if (const auto* URPC = Cast<AUR_BasePlayerController>(UGameplayStatics::GetPlayerController(WorldContext, PlayerIndex)))
	{
		return URPC->GetUserSettings();
	}
	return NULL;
}
