// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Weap_Shotgun.h"

#include "UR_Projectile.h"
#include "UR_FunctionLibrary.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_Weap_Shotgun::AUR_Weap_Shotgun(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    WeaponName = "Shotgun";
    WeaponKnockBackValue = 20.f;

    SpawnBoxes = {
        { FVector(5.0f, 0.0f, 0.0f), FVector(5.f, 15.f, 15.f), 2 },
        { FVector(5.0f, -15.0f, -15.0f), FVector(5.f, 15.f, 15.f), 2 },
        { FVector(5.0f, -15.0f, +15.0f), FVector(5.f, 15.f, 15.f), 2 },
        { FVector(5.0f, +15.0f, -15.0f), FVector(5.f, 15.f, 15.f), 2 },
        { FVector(5.0f, +15.0f, +15.0f), FVector(5.f, 15.f, 15.f), 2 },
    };
    OffsetSpread = 0.2f;
    UseMuzzleDistance = 100.f;

    ShotgunFireMode = CreateDefaultSubobject<UUR_FireModeBasic>(TEXT("ShotgunFireMode"));
}

void AUR_Weap_Shotgun::AuthorityShot_Implementation(UUR_FireModeBasic* FireMode, const FSimulatedShotInfo& SimulatedInfo)
{
    if (FireMode == ShotgunFireMode && FireMode->ProjectileClass)
    {
        FVector FireLoc;
        FRotator FireRot;
        GetValidatedFireVector(SimulatedInfo, FireLoc, FireRot, FireMode->MuzzleSocketName);

        FVector SpreadReferencePoint = FireLoc - UseMuzzleDistance * FireRot.Vector();

        for (const FShotgunSpawnBox& SpawnBox : SpawnBoxes)
        {
            for (int32 j = 0; j < SpawnBox.Count; j++)
            {
                FVector RelOffset(SpawnBox.RelativeLoc);
                RelOffset += UUR_FunctionLibrary::RandomVectorInRange(-SpawnBox.Extent, SpawnBox.Extent);
                FVector SpawnLoc = FireLoc + FireRot.RotateVector(RelOffset);
                FRotator SpawnRot = FMath::Lerp(FireRot, (SpawnLoc - SpreadReferencePoint).Rotation(), OffsetSpread);
                SpawnProjectile(FireMode->ProjectileClass, SpawnLoc, SpawnRot);
            }
        }
    }
    else
    {
        Super::AuthorityShot_Implementation(FireMode, SimulatedInfo);
    }
}
