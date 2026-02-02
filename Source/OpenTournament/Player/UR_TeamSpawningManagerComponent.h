// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Player/UR_PlayerSpawningManagerComponent.h"

#include "UR_TeamSpawningManagerComponent.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AActor;
class AController;
class AUR_PlayerStart;
class UObject;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @class UUR_TeamSpawningManagerComponent
 */
UCLASS()
class UUR_TeamSpawningManagerComponent : public UUR_PlayerSpawningManagerComponent
{
    GENERATED_BODY()

public:
    UUR_TeamSpawningManagerComponent(const FObjectInitializer& ObjectInitializer);

    virtual AActor* OnChoosePlayerStart(AController* Player, TArray<AUR_PlayerStart*>& PlayerStarts) override;
    virtual void OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation) override;

protected:
};
