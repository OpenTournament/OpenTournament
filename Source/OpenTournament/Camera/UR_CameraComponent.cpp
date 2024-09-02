// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_CameraComponent.h"

#include <Engine/Canvas.h>
#include <Engine/Engine.h>
#include <GameFramework/Pawn.h>
#include <GameFramework/PlayerController.h>

#include "UR_CameraMode.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_CameraComponent)

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_CameraComponent::UUR_CameraComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    CameraModeStack = nullptr;
    FieldOfViewOffset = 0.0f;
}

void UUR_CameraComponent::OnRegister()
{
    Super::OnRegister();

    if (!CameraModeStack)
    {
        CameraModeStack = NewObject<UUR_CameraModeStack>(this);
        check(CameraModeStack);
    }
}

void UUR_CameraComponent::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
    check(CameraModeStack);

    UpdateCameraModes();

    FGameCameraModeView CameraModeView;
    CameraModeStack->EvaluateStack(DeltaTime, CameraModeView);

    // Keep player controller in sync with the latest view.
    if (APawn* TargetPawn = Cast<APawn>(GetTargetActor()))
    {
        if (APlayerController* PC = TargetPawn->GetController<APlayerController>())
        {
            PC->SetControlRotation(CameraModeView.ControlRotation);
        }
    }

    // Apply any offset that was added to the field of view.
    CameraModeView.FieldOfView += FieldOfViewOffset;
    FieldOfViewOffset = 0.0f;

    // Keep camera component in sync with the latest view.
    SetWorldLocationAndRotation(CameraModeView.Location, CameraModeView.Rotation);
    FieldOfView = CameraModeView.FieldOfView;

    // Fill in desired view.
    DesiredView.Location = CameraModeView.Location;
    DesiredView.Rotation = CameraModeView.Rotation;
    DesiredView.FOV = CameraModeView.FieldOfView;
    DesiredView.OrthoWidth = OrthoWidth;
    DesiredView.OrthoNearClipPlane = OrthoNearClipPlane;
    DesiredView.OrthoFarClipPlane = OrthoFarClipPlane;
    DesiredView.AspectRatio = AspectRatio;
    DesiredView.bConstrainAspectRatio = bConstrainAspectRatio;
    DesiredView.bUseFieldOfViewForLOD = bUseFieldOfViewForLOD;
    DesiredView.ProjectionMode = ProjectionMode;

    // See if the CameraActor wants to override the PostProcess settings used.
    DesiredView.PostProcessBlendWeight = PostProcessBlendWeight;
    if (PostProcessBlendWeight > 0.0f)
    {
        DesiredView.PostProcessSettings = PostProcessSettings;
    }

    if (IsXRHeadTrackedCamera())
    {
        // In XR much of the camera behavior above is irrellevant, but the post process settings are not.
        Super::GetCameraView(DeltaTime, DesiredView);
    }
}

void UUR_CameraComponent::UpdateCameraModes()
{
    check(CameraModeStack);

    if (CameraModeStack->IsStackActivate())
    {
        if (DetermineCameraModeDelegate.IsBound())
        {
            if (const TSubclassOf<UUR_CameraMode> CameraMode = DetermineCameraModeDelegate.Execute())
            {
                CameraModeStack->PushCameraMode(CameraMode);
            }
        }
    }
}

void UUR_CameraComponent::DrawDebug(UCanvas* Canvas) const
{
    check(Canvas);

    FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

    DisplayDebugManager.SetFont(GEngine->GetSmallFont());
    DisplayDebugManager.SetDrawColor(FColor::Yellow);
    DisplayDebugManager.DrawString(FString::Printf(TEXT("GameCameraComponent: %s"), *GetNameSafe(GetTargetActor())));

    DisplayDebugManager.SetDrawColor(FColor::White);
    DisplayDebugManager.DrawString(FString::Printf(TEXT("   Location: %s"), *GetComponentLocation().ToCompactString()));
    DisplayDebugManager.DrawString(FString::Printf(TEXT("   Rotation: %s"), *GetComponentRotation().ToCompactString()));
    DisplayDebugManager.DrawString(FString::Printf(TEXT("   FOV: %f"), FieldOfView));

    check(CameraModeStack);
    CameraModeStack->DrawDebug(Canvas);
}

void UUR_CameraComponent::GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const
{
    check(CameraModeStack);
    CameraModeStack->GetBlendInfo(/*out*/ OutWeightOfTopLayer, /*out*/ OutTagOfTopLayer);
}
