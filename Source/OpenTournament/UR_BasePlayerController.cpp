// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_BasePlayerController.h"

#include "GameMapsSettings.h"
#include "Kismet/GameplayStatics.h"

#include "UR_PlayerInput.h"
#include "UR_UserSettings.h"
#include "UR_MPC_Global.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_BasePlayerController::AUR_BasePlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ConfiguredFOV = 90;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_BasePlayerController::InitInputSystem()
{
	InitUserSettings();	//Sounds like a good place to do this. Not sure where else.

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

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_BasePlayerController::InitUserSettings()
{
	if (!UserSettings)
	{
		UserSettings = NewObject<UUR_UserSettings>(this);
		ApplyAllSettings();
	}
}

void AUR_BasePlayerController::ApplyAllSettings()
{
	ApplyTeamColorSettings();
}

void AUR_BasePlayerController::ApplyTeamColorSettings()
{
	if (const auto Settings = GetUserSettings())
	{
		const auto Params = GetDefault<UUR_MPC_Global>();
		UUR_MPC_Global::SetVector(this, Params->P_AllyColor, Settings->AllyColor);
		UUR_MPC_Global::SetVector(this, Params->P_EnemyColor, Settings->EnemyColor);
		UUR_MPC_Global::SetVector(this, Params->P_EnemyColor2, Settings->EnemyColor2);
		UUR_MPC_Global::SetVector(this, Params->P_EnemyColor3, Settings->EnemyColor3);
	}
}
