// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "IndicatorDescriptor.h"

#include "Engine/LocalPlayer.h"
#include "SceneView.h"

#include "UI/IndicatorSystem/UR_IndicatorManagerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(IndicatorDescriptor)

/////////////////////////////////////////////////////////////////////////////////////////////////

bool FIndicatorProjection::Project(const UIndicatorDescriptor& IndicatorDescriptor, const FSceneViewProjectionData& InProjectionData, const FVector2f& ScreenSize, FVector& OutScreenPositionWithDepth)
{
    if (USceneComponent* Component = IndicatorDescriptor.GetSceneComponent())
    {
        TOptional<FVector> WorldLocation;
        if (IndicatorDescriptor.GetComponentSocketName() != NAME_None)
        {
            WorldLocation = Component->GetSocketTransform(IndicatorDescriptor.GetComponentSocketName()).GetLocation();
        }
        else
        {
            WorldLocation = Component->GetComponentLocation();
        }

        const FVector ProjectWorldLocation = WorldLocation.GetValue() + IndicatorDescriptor.GetWorldPositionOffset();
        const EActorCanvasProjectionMode ProjectionMode = IndicatorDescriptor.GetProjectionMode();

        switch (ProjectionMode)
        {
            case EActorCanvasProjectionMode::ComponentPoint:
            {
                if (WorldLocation.IsSet())
                {
                    FVector2D OutScreenSpacePosition;
                    const bool bInFrontOfCamera = ULocalPlayer::GetPixelPoint(InProjectionData, ProjectWorldLocation, OutScreenSpacePosition, &ScreenSize);

                    OutScreenSpacePosition.X += IndicatorDescriptor.GetScreenSpaceOffset().X * (bInFrontOfCamera ? 1 : -1);
                    OutScreenSpacePosition.Y += IndicatorDescriptor.GetScreenSpaceOffset().Y;

                    if (!bInFrontOfCamera && FBox2f(FVector2f::Zero(), ScreenSize).IsInside((FVector2f)OutScreenSpacePosition))
                    {
                        const FVector2f CenterToPosition = (FVector2f(OutScreenSpacePosition) - (ScreenSize / 2)).GetSafeNormal();
                        OutScreenSpacePosition = FVector2D((ScreenSize / 2) + CenterToPosition * ScreenSize);
                    }

                    OutScreenPositionWithDepth = FVector(OutScreenSpacePosition.X, OutScreenSpacePosition.Y, FVector::Dist(InProjectionData.ViewOrigin, ProjectWorldLocation));

                    return true;
                }

                return false;
            }
            case EActorCanvasProjectionMode::ComponentScreenBoundingBox:
            case EActorCanvasProjectionMode::ActorScreenBoundingBox:
            {
                FBox IndicatorBox;
                if (ProjectionMode == EActorCanvasProjectionMode::ActorScreenBoundingBox)
                {
                    IndicatorBox = Component->GetOwner()->GetComponentsBoundingBox();
                }
                else
                {
                    IndicatorBox = Component->Bounds.GetBox();
                }

                FVector2D LL, UR;
                const bool bInFrontOfCamera = ULocalPlayer::GetPixelBoundingBox(InProjectionData, IndicatorBox, LL, UR, &ScreenSize);

                const FVector& BoundingBoxAnchor = IndicatorDescriptor.GetBoundingBoxAnchor();
                const FVector2D& ScreenSpaceOffset = IndicatorDescriptor.GetScreenSpaceOffset();

                FVector ScreenPositionWithDepth;
                ScreenPositionWithDepth.X = FMath::Lerp(LL.X, UR.X, BoundingBoxAnchor.X) + ScreenSpaceOffset.X * (bInFrontOfCamera ? 1 : -1);
                ScreenPositionWithDepth.Y = FMath::Lerp(LL.Y, UR.Y, BoundingBoxAnchor.Y) + ScreenSpaceOffset.Y;
                ScreenPositionWithDepth.Z = FVector::Dist(InProjectionData.ViewOrigin, ProjectWorldLocation);

                const FVector2f ScreenSpacePosition = FVector2f(FVector2D(ScreenPositionWithDepth));
                if (!bInFrontOfCamera && FBox2f(FVector2f::Zero(), ScreenSize).IsInside(ScreenSpacePosition))
                {
                    const FVector2f CenterToPosition = (ScreenSpacePosition - (ScreenSize / 2)).GetSafeNormal();
                    const FVector2f ScreenPositionFromBehind = (ScreenSize / 2) + CenterToPosition * ScreenSize;
                    ScreenPositionWithDepth.X = ScreenPositionFromBehind.X;
                    ScreenPositionWithDepth.Y = ScreenPositionFromBehind.Y;
                }

                OutScreenPositionWithDepth = ScreenPositionWithDepth;
                return true;
            }
            case EActorCanvasProjectionMode::ActorBoundingBox:
            case EActorCanvasProjectionMode::ComponentBoundingBox:
            {
                FBox IndicatorBox;
                if (ProjectionMode == EActorCanvasProjectionMode::ActorBoundingBox)
                {
                    IndicatorBox = Component->GetOwner()->GetComponentsBoundingBox();
                }
                else
                {
                    IndicatorBox = Component->Bounds.GetBox();
                }

                const FVector ProjectBoxPoint = IndicatorBox.GetCenter() + (IndicatorBox.GetSize() * (IndicatorDescriptor.GetBoundingBoxAnchor() - FVector(0.5)));

                FVector2D OutScreenSpacePosition;
                const bool bInFrontOfCamera = ULocalPlayer::GetPixelPoint(InProjectionData, ProjectBoxPoint, OutScreenSpacePosition, &ScreenSize);
                OutScreenSpacePosition.X += IndicatorDescriptor.GetScreenSpaceOffset().X * (bInFrontOfCamera ? 1 : -1);
                OutScreenSpacePosition.Y += IndicatorDescriptor.GetScreenSpaceOffset().Y;

                if (!bInFrontOfCamera && FBox2f(FVector2f::Zero(), ScreenSize).IsInside((FVector2f)OutScreenSpacePosition))
                {
                    const FVector2f CenterToPosition = (FVector2f(OutScreenSpacePosition) - (ScreenSize / 2)).GetSafeNormal();
                    OutScreenSpacePosition = FVector2D((ScreenSize / 2) + CenterToPosition * ScreenSize);
                }

                OutScreenPositionWithDepth = FVector(OutScreenSpacePosition.X, OutScreenSpacePosition.Y, FVector::Dist(InProjectionData.ViewOrigin, ProjectBoxPoint));

                return true;
            }
        }
    }

    return false;
}

void UIndicatorDescriptor::SetIndicatorManagerComponent(UUR_IndicatorManagerComponent* InManager)
{
    // Make sure nobody has set this.
    if (ensure(ManagerPtr.IsExplicitlyNull()))
    {
        ManagerPtr = InManager;
    }
}

void UIndicatorDescriptor::UnregisterIndicator()
{
    if (UUR_IndicatorManagerComponent* Manager = ManagerPtr.Get())
    {
        Manager->RemoveIndicator(this);
    }
}
