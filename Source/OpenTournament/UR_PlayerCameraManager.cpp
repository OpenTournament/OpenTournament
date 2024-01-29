// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_PlayerCameraManager.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"

#include "UR_BasePlayerController.h"
#include "UR_Character.h"
#include "UR_FunctionLibrary.h"

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

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_PlayerCameraManager::GetCameraViewPoint_Direct(FVector& OutCamLoc, FRotator& OutCamRot)
{
    if (auto CameraComponent = GetCurrentCameraComponent())
    {
        OutCamLoc = CameraComponent->GetComponentLocation();
        OutCamRot = CameraComponent->GetComponentRotation();
        return;
    }

    // If using fancy camera styles provided in PlayerCameraMaanger, can't help much with that.
    if (CameraStyle == FName(TEXT("Default")))
    {
        if (auto Actor = GetViewTarget())
        {
            // PlayerController case
            if (auto PC = Cast<APlayerController>(Actor))
            {
                OutCamLoc = PC->GetFocalLocation();
                OutCamRot = PC->GetControlRotation();
                return;
            }

            // Default AActor case without camera component
            Actor->GetActorEyesViewPoint(OutCamLoc, OutCamRot);
            return;
        }
    }

    // fallback
    GetCameraViewPoint(OutCamLoc, OutCamRot);
}

UCameraComponent* AUR_PlayerCameraManager::GetCurrentCameraComponent()
{
    // See APlayerCameraManager::UpdateViewTarget - we're gonna follow the same code path it uses

    // If using fancy camera styles, there's no camera component.
    // In "Default" camera style, we enter into UpdateViewTargetInternal, which calls CalcCamera() on the target actor.
    if (CameraStyle == FName(TEXT("Default")))
    {
        // There must be a viewtarget
        if (auto Actor = GetViewTarget())
        {
            // Camera Actor case (note: engine does not check if its CameraComponent is active)
            if (auto CamActor = Cast<ACameraActor>(Actor))
            {
                return CamActor->GetCameraComponent();
            }

            // URCharacter case
            if (auto URCharacter = Cast<AUR_Character>(Actor))
            {
                return URCharacter->PickCamera();
            }

            // PlayerController case
            if (auto PC = Cast<APlayerController>(Actor))
            {
                return NULL;
            }

            // Default AActor case
            if (Actor->bFindCameraComponentWhenViewTarget)
            {
                TInlineComponentArray<UCameraComponent*> ActorCameras;
                Actor->GetComponents<UCameraComponent>(ActorCameras);
                for (UCameraComponent* CamComp : ActorCameras)
                {
                    if (CamComp->IsActive())
                    {
                        return CamComp;
                    }
                }
            }
        }
    }
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_PlayerCameraManager* AUR_PlayerCameraManager::UR_GetLocalPlayerCameraManager(const UObject* WorldContext)
{
    if (auto PC = UUR_FunctionLibrary::GetLocalPC<APlayerController>(WorldContext))
    {
        return Cast<AUR_PlayerCameraManager>(PC->PlayerCameraManager);
    }
    return NULL;
}

AUR_PlayerCameraManager* AUR_PlayerCameraManager::UR_GetPlayerCameraManager(const APlayerController* InPlayerController)
{
    if (InPlayerController)
    {
        return Cast<AUR_PlayerCameraManager>(InPlayerController->PlayerCameraManager);
    }
    return NULL;
}
