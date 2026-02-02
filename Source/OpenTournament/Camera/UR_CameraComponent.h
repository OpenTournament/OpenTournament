// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Camera/CameraComponent.h>
#include <GameFramework/Actor.h>

#include "UR_CameraComponent.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UCanvas;
class UUR_CameraMode;
class UUR_CameraModeStack;
class UObject;
struct FFrame;
struct FGameplayTag;
struct FMinimalViewInfo;

/////////////////////////////////////////////////////////////////////////////////////////////////

template <class TClass>
class TSubclassOf;
DECLARE_DELEGATE_RetVal(TSubclassOf<UUR_CameraMode>, FGameCameraModeDelegate);

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * UUR_CameraComponent
 *
 *	The base camera component class used by this project.
 */
UCLASS()
class UUR_CameraComponent : public UCameraComponent
{
    GENERATED_BODY()

public:
    UUR_CameraComponent(const FObjectInitializer& ObjectInitializer);

    // Returns the camera component if one exists on the specified actor.
    UFUNCTION(BlueprintPure, Category = "OT|Camera")
    static UUR_CameraComponent* FindCameraComponent(const AActor* Actor)
    {
        return (Actor ? Actor->FindComponentByClass<UUR_CameraComponent>() : nullptr);
    }

    // Returns the target actor that the camera is looking at.
    virtual AActor* GetTargetActor() const
    {
        return GetOwner();
    }

    // Delegate used to query for the best camera mode.
    FGameCameraModeDelegate DetermineCameraModeDelegate;

    // Add an offset to the field of view.  The offset is only for one frame, it gets cleared once it is applied.
    void AddFieldOfViewOffset(float FovOffset)
    {
        FieldOfViewOffset += FovOffset;
    }

    virtual void DrawDebug(UCanvas* Canvas) const;

    // Gets the tag associated with the top layer and the blend weight of it
    void GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const;

protected:
    virtual void OnRegister() override;

    virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;

    virtual void UpdateCameraModes();

protected:
    // Stack used to blend the camera modes.
    UPROPERTY()
    TObjectPtr<UUR_CameraModeStack> CameraModeStack;

    // Offset applied to the field of view.  The offset is only for one frame, it gets cleared once it is applied.
    float FieldOfViewOffset;
};
