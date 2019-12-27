// Copyright 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_CheatManager.h"

#include "UR_PlayerController.h"
#include "UR_PlayerState.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_CheatManager::Cheat_AddScore(int32 n)
{
    if (auto PC = Cast<AUR_PlayerController>(GetOuter()))
    {
        if (auto PS = Cast<AUR_PlayerState>(PC->PlayerState))
        {
            PS->Score += n;
        }
    }
}
