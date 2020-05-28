// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

#include "UR_FireModeZoom.h"

#include "UnrealNetwork.h"
#include "UserWidget.h"
#include "WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"

#include "UR_FunctionLibrary.h"
#include "UR_Character.h"

void UUR_FireModeZoom::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    //TODO: this should only replicate to spectators if possible
    DOREPLIFETIME_CONDITION(UUR_FireModeZoom, bZooming, COND_SkipOwner);
}

template<class T> T* UUR_FireModeZoom::GetInstigator()
{
    if (GetOwner())
    {
        return GetOwner()->GetInstigator<T>();
    }
    return NULL;
}

void UUR_FireModeZoom::RequestStartFire_Implementation()
{
    // Toggle zoom by registering/unregistering it in character
    if (AUR_Character* URChar = GetInstigator<AUR_Character>())
    {
        bool bActivate = (URChar->CurrentZoomInterface != this);
        URChar->RegisterZoomInterface(bActivate ? this : NULL);
    }
}

void UUR_FireModeZoom::Deactivate()
{
    Super::Deactivate();

    // Deactivate zoom when the component is deactivated
    if (AUR_Character* URChar = GetInstigator<AUR_Character>())
    {
        URChar->RegisterZoomInterface(NULL);
    }
}

void UUR_FireModeZoom::AIF_InternalActivate_Implementation()
{
    if (ZoomWidget)
    {
        return;
    }

    if (ZoomWidgetClass)
    {
        ZoomWidget = UWidgetBlueprintLibrary::Create(this, ZoomWidgetClass, GetWorld()->GetFirstPlayerController());
        if (ZoomWidget)
        {
            //TODO: define what goes in PlayerScreen, what goes in viewport, and zIndex ranges...?
            ZoomWidget->AddToPlayerScreen(1);
        }
    }

    UGameplayStatics::PlaySound2D(this, ZoomInSound);

    //TBD: this might be better in Character class? dunno
    if (bHide1PMeshes)
    {
        AUR_Character* URChar = GetInstigator<AUR_Character>();
        if (URChar && URChar->GetMesh1P())
        {
            URChar->GetMesh1P()->SetVisibility(false, true);
        }
    }

    AnimTarget = 1.f;
    AnimDur = ZoomInTime * (1.f - AnimAlpha);
    if (AnimDur > 0.f)
    {
        SetComponentTickEnabled(true);
    }
    else
    {
        SetComponentTickEnabled(false);
        AnimAlpha = AnimTarget;
        UpdateZoom();
    }
}

void UUR_FireModeZoom::AIF_InternalDeactivate_Implementation()
{
    if (ZoomWidget)
    {
        ZoomWidget->RemoveFromParent();
        ZoomWidget = NULL;
    }

    UGameplayStatics::PlaySound2D(this, ZoomOutSound);

    if (bHide1PMeshes)
    {
        AUR_Character* URChar = GetInstigator<AUR_Character>();
        if (URChar && URChar->GetMesh1P())
        {
            //TODO: This might clash with "hidden weapon" setting when we have one
            URChar->GetMesh1P()->SetVisibility(true, true);
        }
    }

    AnimTarget = 0.f;
    AnimDur = ZoomOutTime * AnimAlpha;
    if (AnimDur > 0.f)
    {
        SetComponentTickEnabled(true);
    }
    else
    {
        SetComponentTickEnabled(false);
        AnimAlpha = AnimTarget;
        UpdateZoom();
    }
}

void UUR_FireModeZoom::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (FMath::Abs(AnimAlpha - AnimTarget) < 0.01f)
    {
        AnimAlpha = AnimTarget;
        SetComponentTickEnabled(false);
    }
    else if (AnimTarget > AnimAlpha)
    {
        AnimAlpha = FMath::Min(AnimTarget, AnimAlpha + DeltaTime / AnimDur);
    }
    else
    {
        AnimAlpha = FMath::Max(AnimTarget, AnimAlpha - DeltaTime / AnimDur);
    }

    UpdateZoom();
}


void UUR_FireModeZoom::UpdateZoom()
{
    /**
    * TODO: define how our FOV is going to work.
    *
    * There are three levels of camera management :
    * CameraActor <- Pawn/Controller <- PlayerCameraManager
    *
    * https://docs.unrealengine.com/en-US/Gameplay/Framework/Camera/index.html
    *
    * In base engine, PlayerController's FOV exec command sets a LockedFOV on PlayerCameraManager.
    * Once a FOV is locked in, CameraActor's FOV is not used anymore.
    */

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC && PC->PlayerCameraManager)
    {
        float DefaultFOV = PC->PlayerCameraManager->DefaultFOV;
        //float CurrentFOV = PC->PlayerCameraManager->GetFOVAngle();
        float NewFOV;
        if (AnimTarget > 0.f)
        {
            NewFOV = FMath::InterpEaseOut(DefaultFOV, ZoomFOV, AnimAlpha, 2.f);
        }
        else
        {
            NewFOV = FMath::InterpEaseOut(ZoomFOV, DefaultFOV, 1.f - AnimAlpha, 2.f);
        }
        PC->PlayerCameraManager->SetFOV(NewFOV);
    }

    /*
    AUR_Character* URChar = Cast<AUR_Character>(GetWorld()->GetFirstPlayerController()->GetPawn());
    if (URChar && URChar->CharacterCameraComponent)
    {
        URChar->CharacterCameraComponent->SetFieldOfView();
    }
    */
}

//============================================================
// Replication
//============================================================

void UUR_FireModeZoom::ServerSetZoomState_Implementation(bool bNewZooming)
{
    bZooming = bNewZooming;
}

void UUR_FireModeZoom::OnRep_bZooming()
{
    if (AUR_Character* URChar = GetInstigator<AUR_Character>())
    {
        URChar->RegisterZoomInterface(bZooming ? this : NULL);
    }
}
