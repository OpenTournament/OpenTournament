// Copyright Epic Games, Inc. All Rights Reserved.

#include "ModularGameMode.h"

#include "ModularGameState.h"
#include "ModularPawn.h"
#include "ModularPlayerController.h"
#include "ModularPlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ModularGameMode)

AModularGameModeBase::AModularGameModeBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GameStateClass = AModularGameStateBase::StaticClass();
	PlayerControllerClass = AModularPlayerController::StaticClass();
	PlayerStateClass = AModularPlayerState::StaticClass();
	DefaultPawnClass = AModularPawn::StaticClass();
}

AModularGameMode::AModularGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GameStateClass = AModularGameState::StaticClass();
	PlayerControllerClass = AModularPlayerController::StaticClass();
	PlayerStateClass = AModularPlayerState::StaticClass();
	DefaultPawnClass = AModularPawn::StaticClass();
}

