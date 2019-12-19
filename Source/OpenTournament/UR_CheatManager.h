// Copyright 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"

#include "UR_CheatManager.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * 
 */
UCLASS()
class OPENTOURNAMENT_API UUR_CheatManager : public UCheatManager
{
    GENERATED_BODY()

    UFUNCTION(exec, Category = "Cheat")
    void Cheat_AddScore(int32 n = 1);
};
