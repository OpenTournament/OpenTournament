// Copyright 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_FunctionLibrary.h"

#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"

#include "UR_GameModeBase.h"
#include "UR_PlayerController.h"
#include "UR_PlayerState.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_GameModeBase* UUR_FunctionLibrary::GetGameModeDefaultObject(const UObject* WorldContextObject)
{
    AUR_GameModeBase* GameModeDefaultObject = nullptr;
    
    if (const auto World = WorldContextObject->GetWorld())
    {
        if (const auto GameState = World->GetGameState())
        {
            if (auto GameModeClass = GameState->GameModeClass)
            {
                GameModeDefaultObject = Cast<AUR_GameModeBase>(GameModeClass->GetDefaultObject());
            }
        }
    }

    return GameModeDefaultObject;
}

int32 UUR_FunctionLibrary::GetPlayerStateValue(APlayerController* PlayerController)
{
    int32 outValue = -1;
    if (PlayerController)
    {
        if (const auto PlayerState = Cast<AUR_PlayerState>(PlayerController->PlayerState))
        {
            outValue = PlayerState->ArbitraryValue;
        }
    }
    return outValue;
}

FColor UUR_FunctionLibrary::GetPlayerDisplayTextColor(APlayerState* PS)
{
	if (!PS)
	{
		return FColorList::Green;
	}
	else if (PS->bOnlySpectator)
	{
		return GetSpectatorDisplayTextColor();
	}
	else
	{
		AUR_PlayerState* URPS = Cast<AUR_PlayerState>(PS);
		if (URPS)
		{
			//TODO: if team game, return team color, something like URPS->Team->GetDisplayTextColor();

			//TODO: if non team game, return player's color ? if any ? or white ?

			return FColorList::Red;
		}
		else
		{
			return FColorList::Green;	//???
		}
	}
}
