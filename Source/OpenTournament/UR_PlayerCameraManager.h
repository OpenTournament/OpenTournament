// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "UR_PlayerCameraManager.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * 
 */
UCLASS()
class OPENTOURNAMENT_API AUR_PlayerCameraManager : public APlayerCameraManager
{
    GENERATED_BODY()

public:

    virtual void AssignViewTarget(AActor* NewTarget, FTViewTarget& VT, struct FViewTargetTransitionParams TransitionParams = FViewTargetTransitionParams()) override;
};
