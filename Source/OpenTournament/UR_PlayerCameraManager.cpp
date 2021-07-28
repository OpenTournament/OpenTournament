// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_PlayerCameraManager.h"

#include "Engine/World.h"
#include "UR_BasePlayerController.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_PlayerCameraManager::AssignViewTarget(AActor* NewTarget, FTViewTarget& VT, struct FViewTargetTransitionParams TransitionParams)
{
    AActor* OldViewTarget = VT.Target;

    Super::AssignViewTarget(NewTarget, VT, TransitionParams);

    // NOTE: avoid triggering events when World is tearing down, BPs cannot check this and shit goes nuts.
    if (VT.Target != OldViewTarget && !GetWorld()->bIsTearingDown)
    {
        if (auto PC = Cast<AUR_BasePlayerController>(PCOwner))
        {
            PC->OnViewTargetChanged.Broadcast(PC, VT.Target, OldViewTarget);
        }
    }
}
