// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <UR_Weapon.h>

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

    FShotgunSpawnBox()
        : RelativeLoc(0, 0, 0)
        , Extent(0, 0, 0)
        , Count(0)
    {
    }

    FShotgunSpawnBox(FVector RelativeLoc, FVector Extent, int32 Count)
        : RelativeLoc(RelativeLoc)
        , Extent(Extent)
        , Count(Count)
    {
    }
};

/**
 *
 */
UCLASS()
class OPENTOURNAMENT_API AUR_Weap_Shotgun
    : public AUR_Weapon
{
    GENERATED_BODY()

    AUR_Weap_Shotgun(const FObjectInitializer& ObjectInitializer);

public:
    UPROPERTY(EditAnywhere, Category = "Weapon|Shotgun")
    TArray<FShotgunSpawnBox> SpawnBoxes;

    /** Used with MuzzleDistance to compute rockets orientation according to their initial offset */
    UPROPERTY(EditAnywhere, Category = "Weapon|Shotgun")
    float OffsetSpread;

    /** For best results, this value should reflect the average distance from camera to weapon muzzle */
    UPROPERTY(EditAnywhere, Category = "Weapon|Shotgun")
    float UseMuzzleDistance;

    UPROPERTY(VisibleAnywhere)
    UUR_FireModeBasic* ShotgunFireMode;

    virtual void AuthorityShot_Implementation(UUR_FireModeBasic* FireMode, const FSimulatedShotInfo& SimulatedInfo) override;
};
