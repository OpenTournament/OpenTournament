// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Weap_Shotgun.h"

#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"

#include "UR_Character.h"
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
}

void AUR_Weap_Shotgun::SpawnShot_Projectile()
{
    FVector FireLoc;
    FRotator FireRot;
    GetFireVector(FireLoc, FireRot);

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = GetOwner();
    SpawnParams.Instigator = GetInstigator() ? GetInstigator() : Cast<APawn>(GetOwner());
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    const FVector CameraLoc = PlayerController->CharacterCameraComponent->GetComponentLocation();

    for (const FShotgunSpawnBox& SpawnBox : SpawnBoxes)
    {
        for (int32 j = 0; j < SpawnBox.Count; j++)
        {
            FVector RelOffset(SpawnBox.RelativeLoc);
            RelOffset += UUR_FunctionLibrary::RandomVectorInRange(-SpawnBox.Extent, SpawnBox.Extent);

            FVector SpawnLoc = FireLoc + FireRot.RotateVector(RelOffset);

            FRotator MinimumDir = FireRot;
            FRotator MaximumDir = (SpawnLoc - CameraLoc).Rotation();
            FRotator SpawnRot = FMath::Lerp(MinimumDir, MaximumDir, Spread);

            AUR_Projectile* Projectile = GetWorld()->SpawnActor<AUR_Projectile>(ProjectileClass, SpawnLoc, SpawnRot, SpawnParams);
            if (Projectile)
            {
                Projectile->FireAt(SpawnRot.Vector());
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to spawn projectile ??"));
            }
        }
    }
}
