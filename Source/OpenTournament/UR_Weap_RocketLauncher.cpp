// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Weap_RocketLauncher.h"

#include "UR_Projectile.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

// Sets default values
AUR_Weap_RocketLauncher::AUR_Weap_RocketLauncher(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    WeaponName = "Rocket Launcher";

    ChargedFireMode = CreateDefaultSubobject<UUR_FireModeCharged>(TEXT("ChargedFireMode"));
    ChargedFireMode->Index = 1;
    ChargedFireMode->MaxChargeLevel = 3;
    ChargedFireMode->ChargeInterval = 0.9f;     //BP
    ChargedFireMode->MaxChargeHoldTime = 0.5f;   //BP

    RocketsOffset = 5.f;
    DoubleSpread = 1.2f;
    TripleSpread = 2.0f;
    UseMuzzleDistance = 100.f;
}

void AUR_Weap_RocketLauncher::AuthorityShot_Implementation(UUR_FireModeBasic* FireMode, const FSimulatedShotInfo& SimulatedInfo)
{
    if (FireMode == ChargedFireMode && FireMode->ProjectileClass)
    {
        FVector FireLoc;
        FRotator FireRot;
        GetValidatedFireVector(SimulatedInfo, FireLoc, FireRot, FireMode->MuzzleSocketName);

        // Centered rocket
        if (ChargedFireMode->ChargeLevel != 2)
        {
            SpawnProjectile(FireMode->ProjectileClass, FireLoc, FireRot);
        }

        // Spread rockets
        if (ChargedFireMode->ChargeLevel > 1)
        {
            FVector SpreadReferencePoint = FireLoc - UseMuzzleDistance * FireRot.Vector();
            float Spread = (ChargedFireMode->ChargeLevel == 2) ? DoubleSpread : TripleSpread;
            for (int32 DirY = -1; DirY <= 1; DirY += 2)
            {
                FVector RelOffset(0.f, DirY * RocketsOffset, 0.f);
                FVector NewLoc = FireLoc + FireRot.RotateVector(RelOffset);
                FRotator NewRot = FMath::Lerp(FireRot, (NewLoc - SpreadReferencePoint).Rotation(), Spread);
                Super::SpawnProjectile(FireMode->ProjectileClass, NewLoc, NewRot);
            }
        }
    }
    else
    {
        Super::AuthorityShot_Implementation(FireMode, SimulatedInfo);
    }
}
