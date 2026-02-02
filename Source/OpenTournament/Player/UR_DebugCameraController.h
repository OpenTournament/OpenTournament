// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Engine/DebugCameraController.h"

#include "UR_DebugCameraController.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UObject;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * AUR_DebugCameraController
 *
 *	Used for controlling the debug camera when it is enabled via the cheat manager.
 */
UCLASS()
class AUR_DebugCameraController : public ADebugCameraController
{
    GENERATED_BODY()

public:
    AUR_DebugCameraController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
    virtual void AddCheats(bool bForce) override;
};
