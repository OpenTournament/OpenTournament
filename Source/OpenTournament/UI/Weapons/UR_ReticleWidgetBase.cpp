// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_ReticleWidgetBase.h"

#include "Inventory/UR_InventoryItemInstance.h"
#include "Weapons/UR_RangedWeaponInstance.h"
#include "Weapons/UR_WeaponInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_ReticleWidgetBase)

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_ReticleWidgetBase::UUR_ReticleWidgetBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{}

void UUR_ReticleWidgetBase::InitializeFromWeapon(UUR_WeaponInstance* InWeapon)
{
    WeaponInstance = InWeapon;
    InventoryInstance = nullptr;
    if (WeaponInstance)
    {
        InventoryInstance = Cast<UUR_InventoryItemInstance>(WeaponInstance->GetInstigator());
    }
    OnWeaponInitialized();
}


float UUR_ReticleWidgetBase::ComputeSpreadAngle() const
{
    if (const UUR_RangedWeaponInstance* RangedWeapon = Cast<const UUR_RangedWeaponInstance>(WeaponInstance))
    {
        const float BaseSpreadAngle = RangedWeapon->GetCalculatedSpreadAngle();
        const float SpreadAngleMultiplier = RangedWeapon->GetCalculatedSpreadAngleMultiplier();
        const float ActualSpreadAngle = BaseSpreadAngle * SpreadAngleMultiplier;

        return ActualSpreadAngle;
    }
    else
    {
        return 0.0f;
    }
}

bool UUR_ReticleWidgetBase::HasFirstShotAccuracy() const
{
    if (const UUR_RangedWeaponInstance* RangedWeapon = Cast<const UUR_RangedWeaponInstance>(WeaponInstance))
    {
        return RangedWeapon->HasFirstShotAccuracy();
    }
    else
    {
        return false;
    }
}

float UUR_ReticleWidgetBase::ComputeMaxScreenspaceSpreadRadius() const
{
    APlayerController* PC = GetOwningPlayer();
    if (PC && PC->PlayerCameraManager)
    {
        constexpr float LongShotDistance = 10000.f;
        // A weapon's spread can be thought of as a cone shape. To find the screenspace spread for reticle visualization,
        // we create a line on the edge of the cone at a long distance. The end of that point is on the edge of the cone's circle.
        // We then project it back onto the screen. Its distance from screen center is the spread radius.

        // This isn't perfect, due to there being some distance between the camera location and the gun muzzle.

        const float SpreadRadiusRads = FMath::DegreesToRadians(ComputeSpreadAngle() * 0.5f);
        const float SpreadRadiusAtDistance = FMath::Tan(SpreadRadiusRads) * LongShotDistance;

        FVector CamPos;
        FRotator CamOrient;
        PC->PlayerCameraManager->GetCameraViewPoint(CamPos, CamOrient);

        FVector CamForwDir = CamOrient.RotateVector(FVector::ForwardVector);
        FVector CamUpDir = CamOrient.RotateVector(FVector::UpVector);

        FVector OffsetTargetAtDistance = CamPos + (CamForwDir * LongShotDistance) + (CamUpDir * SpreadRadiusAtDistance);

        FVector2D OffsetTargetInScreenspace;

        if (PC->ProjectWorldLocationToScreen(OffsetTargetAtDistance, OffsetTargetInScreenspace, true))
        {
            int32 ViewportSizeX(0), ViewportSizeY(0);
            PC->GetViewportSize(ViewportSizeX, ViewportSizeY);

            const FVector2D ScreenSpaceCenter(FVector::FReal(ViewportSizeX) * 0.5f, FVector::FReal(ViewportSizeY) * 0.5f);

            return (OffsetTargetInScreenspace - ScreenSpaceCenter).Length();
        }
    }

    return 0.0f;
}
