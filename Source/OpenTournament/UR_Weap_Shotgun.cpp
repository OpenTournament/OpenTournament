// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Weap_Shotgun.h"

#include "UR_Projectile.h"
#include "UR_FunctionLibrary.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_Weap_Shotgun::AUR_Weap_Shotgun(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    /*ConstructorHelpers::FObjectFinder<USkeletalMesh> newAsset(TEXT("SkeletalMesh'/Game/SciFiWeapDark/Weapons/Darkness_Shotgun.Darkness_Shotgun'"));
    USkeletalMesh* helper = newAsset.Object;
    Mesh1P->SetSkeletalMesh(helper);*/
    WeaponName = "Shotgun";

    /*ConstructorHelpers::FObjectFinder<USoundCue> newAssetSound(TEXT("SoundCue'/Game/SciFiWeapDark/Sound/Shotgun/Shotgun_Lower_Cue.Shotgun_Lower_Cue'"));
    USoundCue* helperSound;
    helperSound = newAssetSound.Object;
    Sound->SetSound(helperSound);*/

    AmmoName = "Shotgun";

    SpawnBoxes = {
        { FVector(5.0f, 0.0f, 0.0f), FVector(5.f, 15.f, 15.f), 2 },
        { FVector(5.0f, -15.0f, -15.0f), FVector(5.f, 15.f, 15.f), 2 },
        { FVector(5.0f, -15.0f, +15.0f), FVector(5.f, 15.f, 15.f), 2 },
        { FVector(5.0f, +15.0f, -15.0f), FVector(5.f, 15.f, 15.f), 2 },
        { FVector(5.0f, +15.0f, +15.0f), FVector(5.f, 15.f, 15.f), 2 },
    };
    Spread = 0.2f;

    UseMuzzleDistance = 100.f;

    ShotgunFireMode = CreateDefaultSubobject<UUR_FireModeBasic>(TEXT("ShotgunFireMode"));
}

void AUR_Weap_Shotgun::AuthorityShot_Implementation(UUR_FireModeBasic* FireMode, const FSimulatedShotInfo& SimulatedInfo)
{
    if (FireMode != ShotgunFireMode)
    {
        Super::AuthorityShot_Implementation(FireMode, SimulatedInfo);
        return;
    }

    if (FireMode->ProjectileClass)
    {
        //TODO: validate passed in fire location
        FVector FireLoc = SimulatedInfo.Vectors[0];

        // Fire direction doesn't need validation
        const FVector& FireDir = SimulatedInfo.Vectors[1];
        FRotator FireRot = FireDir.Rotation();

        FVector SpreadReferencePoint = FireLoc - UseMuzzleDistance * FireDir;

        for (const FShotgunSpawnBox& SpawnBox : SpawnBoxes)
        {
            for (int32 j = 0; j < SpawnBox.Count; j++)
            {
                FVector RelOffset(SpawnBox.RelativeLoc);
                RelOffset += UUR_FunctionLibrary::RandomVectorInRange(-SpawnBox.Extent, SpawnBox.Extent);

                FVector SpawnLoc = FireLoc + FireRot.RotateVector(RelOffset);

                FRotator MinimumDir = FireRot;
                FRotator MaximumDir = (SpawnLoc - SpreadReferencePoint).Rotation();
                FRotator SpawnRot = FMath::Lerp(MinimumDir, MaximumDir, Spread);

                SpawnProjectile(FireMode->ProjectileClass, SpawnLoc, SpawnRot);
            }
        }
    }
}
