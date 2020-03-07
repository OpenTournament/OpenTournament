// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "UR_Weapon.h"

#include "UR_Weap_Shotgun.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
* Defines an area of random spawn for one or more bullet.
*/
USTRUCT(BlueprintType)
struct FShotgunSpawnBox
{
    GENERATED_BODY()

    /**
    * Center of the box.
    * Relative offset from the default centered fire location.
    * X = forward.
    */
    UPROPERTY(EditAnywhere)
    FVector RelativeLoc;

    /**
    * Extent of the box.
    * Randomness is generated between -Extent and +Extent.
    */
    UPROPERTY(EditAnywhere)
    FVector Extent;

    /**
    * Amount of bullets to spawn in this box.
    */
    UPROPERTY(EditAnywhere)
    int32 Count;
};

/**
 *
 */
UCLASS()
class OPENTOURNAMENT_API AUR_Weap_Shotgun : public AUR_Weapon
{
    GENERATED_BODY()

    AUR_Weap_Shotgun(const FObjectInitializer& ObjectInitializer);

    UPROPERTY(EditAnywhere, Category = "Weapon|Shotgun")
    TArray<FShotgunSpawnBox> SpawnBoxes;

    UPROPERTY(EditAnywhere, Category = "Weapon|Shotgun")
    float Spread;

    virtual void SpawnShot_Projectile() override;
};
