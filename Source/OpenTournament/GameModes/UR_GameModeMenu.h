// Copyright (c) Open Tournament Games, All Rights Reserved.

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
//UCLASS(Deprecated, meta = (DeprecationMessage="This class to be deprecated"))
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
