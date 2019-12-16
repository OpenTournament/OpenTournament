// Copyright 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Weap_Pistol.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_Weap_Pistol::AUR_Weap_Pistol(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    /*ConstructorHelpers::FObjectFinder<USkeletalMesh> newAsset(TEXT("SkeletalMesh'/Game/SciFiWeapDark/Weapons/Darkness_Pistol.Darkness_Pistol'"));
    USkeletalMesh* helper = newAsset.Object;
    Mesh1P->SetSkeletalMesh(helper);*/
    WeaponName = "Pistol";

    /*ConstructorHelpers::FObjectFinder<USoundCue> newAssetSound(TEXT("SoundCue'/Game/SciFiWeapDark/Sound/Pistol/Pistol_Lower_Cue.Pistol_Lower_Cue'"));
    USoundCue* helperSound;
    helperSound = newAssetSound.Object;
    Sound->SetSound(helperSound);*/

    AmmoName = "Pistol";
}