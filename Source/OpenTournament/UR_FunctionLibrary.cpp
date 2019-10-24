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

bool UUR_FunctionLibrary::IsKeyMappedToAction(const FKey& Key, FName ActionName)
{
	UInputSettings* Settings = UInputSettings::GetInputSettings();
	TArray<FInputActionKeyMapping> Mappings;
	Settings->GetActionMappingByName(ActionName, Mappings);
	for (const auto& Mapping : Mappings)
	{
		if (Mapping.Key == Key)
			return true;
	}
	return false;
}

bool UUR_FunctionLibrary::IsKeyMappedToAxis(const FKey& Key, FName AxisName, float Direction)
{
	UInputSettings* Settings = UInputSettings::GetInputSettings();
	TArray<FInputAxisKeyMapping> Mappings;
	Settings->GetAxisMappingByName(AxisName, Mappings);
	for (const auto& Mapping : Mappings)
	{
		if (Mapping.Key == Key && (FMath::IsNearlyZero(Direction) || (Direction*Mapping.Scale > 0)))
			return true;
	}
	return false;
}
