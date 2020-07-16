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
	ConfiguredFOV = 90;
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

UUR_PlayerInput* AUR_BasePlayerController::GetPlayerInput() const
{
	return Cast<UUR_PlayerInput>(PlayerInput);
}

void AUR_BasePlayerController::ReturnToMainMenu()
{
	UGameplayStatics::OpenLevel(this, FName(*GetDefault<UGameMapsSettings>()->GetGameDefaultMap()));
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_BasePlayerController::SpawnPlayerCameraManager()
{
	Super::SpawnPlayerCameraManager();

	if (PlayerCameraManager)
	{
		ClampConfiguredFOV();
		PlayerCameraManager->DefaultFOV = ConfiguredFOV;
		PlayerCameraManager->SetFOV(ConfiguredFOV);
	}
}

void AUR_BasePlayerController::SetConfiguredFOV(int32 NewFOV)
{
	ConfiguredFOV = NewFOV;
	ClampConfiguredFOV();
	SaveConfig();

	if (PlayerCameraManager)
	{
		if (PlayerCameraManager->GetFOVAngle() == PlayerCameraManager->DefaultFOV)
		{
			PlayerCameraManager->SetFOV(ConfiguredFOV);
		}
		PlayerCameraManager->DefaultFOV = ConfiguredFOV;
	}
}

void AUR_BasePlayerController::ClampConfiguredFOV()
{
	ConfiguredFOV = FMath::Clamp(ConfiguredFOV, 80, 120);
}
