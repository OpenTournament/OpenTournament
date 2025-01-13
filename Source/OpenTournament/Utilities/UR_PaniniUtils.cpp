// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_PaniniUtils.h"

#include "Components/MeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "UR_MPC_Global.h"
#include "Camera/UR_PlayerCameraManager.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_PaniniUtils::TogglePaniniProjection(USceneComponent* Component, const bool bEnablePanini, const bool bPropagateToChildren)
{
    if (!Component)
    {
        return;
    }

    if (auto WidgetComp = Cast<UWidgetComponent>(Component))
    {
        // NOTE: We cannot do this due to bad design in WidgetComponent.cpp
        // Bug report submitted - don't have any tracker yet.

        // In short, WidgetComponent doesn't support the SetXXXParameterValueOnMaterials API.
        // At BeginPlay, the component's MID hasn't been created yet, and we cannot work around it.
        // The initialization of MID happens a bit later, then it becomes possible to change parameters via Component->GetMaterialInstance().

        // For now I prefer doing nothing here rather than providing inconsistent behavior.
        return;
    }
    else if (const auto MeshComp = Cast<UMeshComponent>(Component))
    {
        MeshComp->SetScalarParameterValueOnMaterials(PaniniParameterName(), bEnablePanini ? 1.f : 0.f);
    }

    // Arrrgh, can't handle
    // We can pass parameter to particle system, but then particle system has to transfer parameter to the materials it is using.
    // This is extremely heavy to work with, not sure if there's a simpler way, such as a way to access a particle system's materials directly.
    /*
    else if (auto ParticleComp = Cast<UParticleSystemComponent>(Component))
    {
        ParticleComp->
    }
    else if (auto NiagaraComp = Cast<UNiagaraComponent>(Component))
    {
        NiagaraComp->
    }
    */

    if (bPropagateToChildren)
    {
        TArray<USceneComponent*> Children;
        Component->GetChildrenComponents(true, Children);
        for (USceneComponent* ChildComponent : Children)
        {
            TogglePaniniProjection(ChildComponent, bEnablePanini, false);
        }
    }
}

FVector UUR_PaniniUtils::CalcPaniniProjection(const UObject* WorldContext, const FVector& WorldPos)
{
    if (const auto CameraManager = AUR_PlayerCameraManager::UR_GetLocalPlayerCameraManager(WorldContext))
    {
        FVector NewPos = CalcMFShrinkWeapon(CameraManager, WorldPos);

        FVector CamLoc;
        FRotator CamRot;
        CameraManager->GetCameraViewPoint_Direct(CamLoc, CamRot);
        const FTransform CameraTransform(CamRot, CamLoc);

        const FPaniniMaterialParameters& Params = UUR_MPC_Global::GetPaniniParameters(CameraManager);

        // We only use the FovCorrection part at this time (Panini.Projection = 0)
        // So I haven't implemented MF_PaniniProjection at all
        NewPos = CameraTransform.InverseTransformPosition(NewPos);
        NewPos.X *= (Params.Scale / FMath::Tan(0.5f * CameraManager->GetFOVAngle() * PI / 180.f));
        return CameraTransform.TransformPosition(NewPos);
    }
    return WorldPos;
}

FVector UUR_PaniniUtils::CalcMFShrinkWeapon(const UObject* WorldContext, const FVector& WorldPos)
{
    if (const auto CameraManager = AUR_PlayerCameraManager::UR_GetLocalPlayerCameraManager(WorldContext))
    {
        const FPaniniMaterialParameters& Params = UUR_MPC_Global::GetPaniniParameters(CameraManager);

        FVector CamLoc;
        FRotator CamRot;
        CameraManager->GetCameraViewPoint_Direct(CamLoc, CamRot);

        FVector Offset = WorldPos - CamLoc;
        Offset = FMath::Lerp
        (
            Offset.GetSafeNormal() * Params.PushMin,
            Offset.GetSafeNormal() * Params.PushMax,
            (Offset.Size() + (FMath::Tan(0.5f * CameraManager->GetFOVAngle() * PI / 180.f) * Params.DistanceBias)) / Params.DistanceNormalize
        );
        return WorldPos + Offset;
    }
    return WorldPos;
}
