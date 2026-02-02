// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_DebugCameraController.h"

#include "UR_CheatManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_DebugCameraController)

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_DebugCameraController::AUR_DebugCameraController(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Use the same cheat class as UR_PlayerController to allow toggling the debug camera through cheats.
    CheatClass = UUR_CheatManager::StaticClass();
}

void AUR_DebugCameraController::AddCheats(bool bForce)
{
    // Mirrors UR_PlayerController's AddCheats() to avoid the player becoming stuck in the debug camera.
#if USING_CHEAT_MANAGER
    Super::AddCheats(true);
#else //#if USING_CHEAT_MANAGER
    Super::AddCheats(bForce);
#endif // #else //#if USING_CHEAT_MANAGER
}
