// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_PlayerCameraManager.h"
#include "UR_BasePlayerController.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_PlayerCameraManager::AssignViewTarget(AActor* NewTarget, FTViewTarget& VT, struct FViewTargetTransitionParams TransitionParams)
{
	AActor* OldViewTarget = VT.Target;

	Super::AssignViewTarget(NewTarget, VT, TransitionParams);

	if (VT.Target != OldViewTarget)
	{
		if (auto PC = Cast<AUR_BasePlayerController>(PCOwner))
		{
			PC->OnViewTargetChanged.Broadcast(PC, VT.Target, OldViewTarget);
		}
	}
}
