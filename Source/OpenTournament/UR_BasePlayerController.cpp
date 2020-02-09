// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_BasePlayerController.h"

#include "GameMapsSettings.h"
#include "Kismet/GameplayStatics.h"

#include "UR_PlayerInput.h"


/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_BasePlayerController::AUR_BasePlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_BasePlayerController::InitInputSystem()
{
	if (PlayerInput == nullptr)
	{
		PlayerInput = NewObject<UUR_PlayerInput>(this);
	}

	Super::InitInputSystem();
}

UUR_PlayerInput* AUR_BasePlayerController::GetPlayerInput()
{
	return Cast<UUR_PlayerInput>(PlayerInput);
}

void AUR_BasePlayerController::ReturnToMainMenu()
{
	UGameplayStatics::OpenLevel(this, FName(*GetDefault<UGameMapsSettings>()->GetGameDefaultMap()));
}
