// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameFramework/CheatManager.h"

#include "UR_BotCheats.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UUR_BotCreationComponent;
class UObject;
struct FFrame;

/////////////////////////////////////////////////////////////////////////////////////////////////

/** Cheats related to bots */
UCLASS(NotBlueprintable)
class UUR_BotCheats final : public UCheatManagerExtension
{
    GENERATED_BODY()

public:
    UUR_BotCheats();

    // Adds a bot player
    UFUNCTION(Exec, BlueprintAuthorityOnly)
    void AddPlayerBot();

    // Removes a random bot player
    UFUNCTION(Exec, BlueprintAuthorityOnly)
    void RemovePlayerBot();

private:
    UUR_BotCreationComponent* GetBotComponent() const;
};
