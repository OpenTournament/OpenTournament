// Copyright 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"

#include "UR_PlayerState.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * 
 */
UCLASS()
class OPENTOURNAMENT_API AUR_PlayerState : public APlayerState
{
    GENERATED_BODY()
    
    AUR_PlayerState();

public:

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PlayerState")
    int32 ArbitraryValue;
};
