// Copyright 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"

#include "UR_GameState.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Delegates

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMatchStateChanged);

/////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * 
 */
UCLASS()
class OPENTOURNAMENT_API AUR_GameState : public AGameState
{
    GENERATED_BODY()

public:

    virtual void OnRep_MatchState() override;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    UPROPERTY(BlueprintAssignable)
    FMatchStateChanged OnMatchStateChanged;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameState")
    TArray<int32> ArbitraryValues;
};
