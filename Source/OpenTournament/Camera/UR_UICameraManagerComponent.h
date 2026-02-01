// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Camera/PlayerCameraManager.h"

#include "UR_UICameraManagerComponent.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AActor;
class AHUD;
class APlayerController;
class AUR_PlayerCameraManager;
class FDebugDisplayInfo;
class UCanvas;
class UObject;

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(Transient, Within=UR_PlayerCameraManager)
class UUR_UICameraManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    static UUR_UICameraManagerComponent* GetComponent(APlayerController* PC);

public:
    UUR_UICameraManagerComponent();
    virtual void InitializeComponent() override;

    bool IsSettingViewTarget() const
    {
        return bUpdatingViewTarget;
    }

    AActor* GetViewTarget() const
    {
        return ViewTarget;
    }

    void SetViewTarget(AActor* InViewTarget, FViewTargetTransitionParams TransitionParams = FViewTargetTransitionParams());

    bool NeedsToUpdateViewTarget() const;
    void UpdateViewTarget(struct FTViewTarget& OutVT, float DeltaTime);

    void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos);

private:
    UPROPERTY(Transient)
    TObjectPtr<AActor> ViewTarget;

    UPROPERTY(Transient)
    bool bUpdatingViewTarget;
};
