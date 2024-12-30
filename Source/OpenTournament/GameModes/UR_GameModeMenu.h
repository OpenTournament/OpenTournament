// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"

#include "UR_GameModeBase.h"

#include "UR_GameModeMenu.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations

// TODO

/////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * GameMode for Menus
 */
UCLASS()
class OPENTOURNAMENT_API AUR_GameModeMenu
    : public AUR_GameModeBase
{
    GENERATED_BODY()

public:
    AUR_GameModeMenu();

    virtual void RestartGame() override;

    virtual void RestartPlayer(AController* Player) override;
};
