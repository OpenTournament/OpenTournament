// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Weap_AssaultRifle.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_Weap_AssaultRifle::AUR_Weap_AssaultRifle(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    /*ConstructorHelpers::FObjectFinder<USkeletalMesh> newAsset(TEXT("SkeletalMesh'/Game/SciFiWeapDark/Weapons/Darkness_AssaultRifle.Darkness_AssaultRifle'"));
    USkeletalMesh* helper = newAsset.Object;
    Mesh1P->SetSkeletalMesh(helper);*/
    WeaponName = "Assault Rifle";
    WeaponKnockBackValue = 30.f;

    /*ConstructorHelpers::FObjectFinder<USoundCue> newAssetSound(TEXT("SoundCue'/Game/SciFiWeapDark/Sound/Rifle/Rifle_Lower_Cue.Rifle_Lower_Cue'"));
    USoundCue* helperSound;
    helperSound = newAssetSound.Object;
    Sound->SetSound(helperSound);*/
}

/////////////////////////////////////////////////////////////////////////////////////////////////
