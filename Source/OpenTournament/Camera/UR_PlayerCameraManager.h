// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Camera/PlayerCameraManager.h>

#include "UR_PlayerCameraManager.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

#define OT_CAMERA_DEFAULT_FOV		(90.0f)
#define OT_CAMERA_DEFAULT_PITCH_MIN	(-89.0f)
#define OT_CAMERA_DEFAULT_PITCH_MAX	(89.0f)

/**
 *
 */
UCLASS(notplaceable)
class OPENTOURNAMENT_API AUR_PlayerCameraManager : public APlayerCameraManager
{
    GENERATED_BODY()

public:

    virtual void AssignViewTarget(AActor* NewTarget, FTViewTarget& VT, struct FViewTargetTransitionParams TransitionParams = FViewTargetTransitionParams()) override;

    /**
    * NOTE: When we want to use CameraViewPoint for gameplay code, chances are we'll be getting a frame(s) old cached value.
    * Spawning effects relative to camera location results in them lagging behind movement/rotation.
    * This is because Camera POV values are the last to be updated in the frame.
    *
    * If we are in PrePhysics, both our objects and the camera are using last frame's positions.
    * If we are in PostPhysics/PostUpdateWork, our objects have been updated but the camera is still old.
    * Besides, we don't get to control SetTimer() or when RPCs are triggered, those will most likely happen before update.
    *
    * A good use case comes from the weapon firing mechanism.
    * Clicking triggers instant fire, but holding fire results in refiring on a timer basis. Firing also triggers on remote client after replication.
    * - Input is handled at start on frame, so when we click, we spawn fire effects in TG_PrePhysics
    * - Experimentation showed that timers trigger during TG_PostUpdateWork (at least for firemodes)
    * - Experimentation showed that RPCs happen during TG_NewlySpawned (whatever that is)
    * Problem here is that for first-person-view, we need to adjust the effect location to account for Panini Projection,
    * which depends on the vector (WeaponMuzzle - CameraLocation).
    * Therefore we get different results based on when stuff is triggered :
    * - In this specific case, PrePhysics is ok because both locations have not been updated yet (last frame) but that vector doesn't really change between frames.
    * - However PostUpdateWork is not ok because Muzzle has been updated but Camera hasn't. Vector is wrong, panini correction is wrong, effect location is wrong.
    *
    * NOTE: I experimented forcing camera update when needed, but that comes with its own set of new issues.
    *
    * NOTE: Do not mistake the camera component with the manager view point.
    *       The camera component updates like all other components, but the manager updates its own view point at end of frame.
    *
    * NOTE: In practice, in first person mode, the manager's view point simply follows the camera component (via the CalcCamera function).
    *
    * Therefore, we're gonna provide GetCameraViewPoint_Direct() which will fetch the camera transform directly, shorting the manager cache.
    * This should fix our specific use-case as the vector will remain consistent whether it is 1-frame old or up to date.
    * This might NOT fix other possible use cases (unknown yet) where 1-frame lag would be an issue.
    */
    UFUNCTION(BlueprintPure)
    void GetCameraViewPoint_Direct(FVector& OutCamLoc, FRotator& OutCamRot);

    /**
    * Utility - Best effort to find the CameraComponent we're currently viewing through, if there is one.
    */
    UFUNCTION(BlueprintPure)
    class UCameraComponent* GetCurrentCameraComponent();

    /**
    * Get local player's camera manager as UR_PlayerCameraManager.
    * Do not use on DedicatedServer.
    */
    UFUNCTION(BlueprintPure, BlueprintCosmetic, Meta = (WorldContextObject = "WorldContext"))
    static AUR_PlayerCameraManager* UR_GetLocalPlayerCameraManager(const UObject* WorldContext);

    /**
    * Get the UR_PlayerCameraManager associated with this player controller.
    */
    UFUNCTION(BlueprintPure)
    static AUR_PlayerCameraManager* UR_GetPlayerCameraManager(const APlayerController* InPlayerController);
};
