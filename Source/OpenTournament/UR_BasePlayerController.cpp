// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_BasePlayerController.h"

#include "GameMapsSettings.h"
#include "Kismet/GameplayStatics.h"

#include "UR_PlayerCameraManager.h"
#include "UR_PlayerInput.h"
#include "UR_UserSettings.h"
#include "UR_MPC_Global.h"
#include "UR_Character.h"
#include "UR_InventoryComponent.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_BasePlayerController::AUR_BasePlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerCameraManagerClass = AUR_PlayerCameraManager::StaticClass();
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_BasePlayerController::PostInitializeComponents()
{
	InitUserSettings();

	Super::PostInitializeComponents();
}

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
		ApplyCameraSettings();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_BasePlayerController::InitUserSettings()
{
	if (!UserSettings && IsLocalPlayerController())
	{
		UserSettings = NewObject<UUR_UserSettings>(this);
		ApplyAllSettings();
	}
}

void AUR_BasePlayerController::ApplyAllSettings()
{
	ApplyCameraSettings();
	ApplyTeamColorSettings();
	ApplyWeaponGroupSettings();
}

void AUR_BasePlayerController::ApplyCameraSettings()
{
	if (const auto Settings = GetUserSettings())
	{
		if (PlayerCameraManager)
		{
			int32 ClampedFOV = FMath::Clamp(Settings->NormalFOV, 80, 120);
			PlayerCameraManager->DefaultFOV = ClampedFOV;
			PlayerCameraManager->SetFOV(ClampedFOV);
		}
	}
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

void AUR_BasePlayerController::ApplyWeaponGroupSettings()
{
	if (auto URCharacter = GetPawn<AUR_Character>())
	{
		URCharacter->SetupWeaponBindings();

		if (URCharacter->InventoryComponent)
		{
			URCharacter->InventoryComponent->RefillWeaponGroups();
		}
	}
}
