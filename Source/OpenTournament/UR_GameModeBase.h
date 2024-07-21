// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <ModularGameMode.h>

#include "UR_GameModeBase.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Base GameMode class for all OpenTournament GameModes
 */
UCLASS()
class OPENTOURNAMENT_API AUR_GameModeBase
    : public AModularGameMode
{
    GENERATED_BODY()

public:
    AUR_GameModeBase();

    /////////////////////////////////////////////////////////////////////////////////////////////////

    TArray<class UUR_ChatComponent*> ChatComponents;

    UFUNCTION(BlueprintCallable, Category = "Chat")
    virtual void RegisterChatComponent(class UUR_ChatComponent* InComponent);

    UFUNCTION(BlueprintCallable, Category = "Chat")
    virtual void UnregisterChatComponent(class UUR_ChatComponent* InComponent);
};
