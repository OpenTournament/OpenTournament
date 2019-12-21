// Copyright 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Weap_RocketLauncher.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

// Sets default values
AUR_Weap_RocketLauncher::AUR_Weap_RocketLauncher(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    /*ConstructorHelpers::FObjectFinder<USkeletalMesh> newAsset(TEXT("SkeletalMesh'/Game/SciFiWeapDark/Weapons/Darkness_RocketLauncher.Darkness_RocketLauncher'"));
    USkeletalMesh *helper = newAsset.Object;
    Mesh1P->SetSkeletalMesh(helper);*/
    WeaponName = "Rocket Launcher";

    /*ConstructorHelpers::FObjectFinder<USoundCue> newAssetSound(TEXT("SoundCue'/Game/SciFiWeapDark/Sound/RocketLauncher/RocketLauncher_Lower_Cue.RocketLauncher_Lower_Cue'"));
    USoundCue* helperSound;
    helperSound = newAssetSound.Object;
    Sound->SetSound(helperSound);*/

    AmmoName = "Rocket";
}