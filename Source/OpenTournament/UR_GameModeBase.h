// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Widgets/UR_Widget_BaseMenu.h"

#include "UR_GameModeBase.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Base GameMode class for all OpenTournament GameModes
 */
UCLASS()
class OPENTOURNAMENT_API AUR_GameModeBase : public AGameMode
{
    GENERATED_BODY()

public:

    AUR_GameModeBase();

    /////////////////////////////////////////////////////////////////////////////////////////////////

    TArray<class UUR_ChatComponent*> ChatComponents;

    UFUNCTION(BlueprintCallable, Category = "Chat")
    virtual void RegisterChatComponent(class UUR_ChatComponent* Comp);

    UFUNCTION(BlueprintCallable, Category = "Chat")
    virtual void UnregisterChatComponent(class UUR_ChatComponent* Comp);
};
