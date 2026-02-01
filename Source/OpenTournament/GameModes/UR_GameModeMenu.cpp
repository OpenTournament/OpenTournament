// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_GameModeMenu.h"

#include "OpenTournament.h"
#include "UR_GameState.h"
#include "UR_PlayerController.h"
#include "UR_PlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_GameModeMenu)

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_GameModeMenu::AUR_GameModeMenu()
{
    //NOTE: we override these to the BP subclasses in the BP subclass
    GameStateClass = AUR_GameState::StaticClass();
    PlayerStateClass = AUR_PlayerState::StaticClass();
    PlayerControllerClass = AUR_PlayerController::StaticClass();
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_GameModeMenu::RestartGame()
{
    return;
}

void AUR_GameModeMenu::RestartPlayer(AController* Player)
{
    return;
}
