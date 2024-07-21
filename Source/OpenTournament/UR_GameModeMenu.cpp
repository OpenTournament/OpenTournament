// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_GameModeMenu.h"
#include "GameFramework/GameState.h"

//UMG
#include "SlateBasics.h"
#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/UMGStyle.h"
#include "Runtime/UMG/Public/Slate/SObjectWidget.h"
#include "Runtime/UMG/Public/IUMGModule.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"

#include "OpenTournament.h"
#include "UR_GameState.h"
#include "UR_PlayerState.h"
#include "UR_PlayerController.h"


/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_GameModeMenu::AUR_GameModeMenu()
{
    //NOTE: we set these to the BP subclasses in the BP subclass so it seems useless here
    GameStateClass = AUR_GameState::StaticClass();
    PlayerStateClass = AUR_PlayerState::StaticClass();
    PlayerControllerClass = AUR_PlayerController::StaticClass();

    MusicComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicComponent"));
    MusicComponent->SetupAttachment(RootComponent);
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
