// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Framework/Commands/InputChord.h"

#include "UR_Type_WeaponGroup.generated.h"

class AUR_Weapon;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
UENUM(BlueprintType)
enum class EWeaponGroupVisibility : uint8
{
    Always,
    NotEmpty,
    Hidden,
};

/**
 *
 */
USTRUCT(BlueprintType)
struct FWeaponGroup
{
    GENERATED_BODY()

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite)
    TArray<TSubclassOf<AUR_Weapon>> WeaponClasses;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite)
    EWeaponGroupVisibility Visibility;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite)
    FInputChord Keybind;

    UPROPERTY(Transient, BlueprintReadOnly)
    TArray<AUR_Weapon*> Weapons;
};
