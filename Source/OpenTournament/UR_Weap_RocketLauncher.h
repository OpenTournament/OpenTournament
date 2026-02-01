// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <UR_Weapon.h>

#include "UR_Weap_RocketLauncher.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
UCLASS()
class OPENTOURNAMENT_API AUR_Weap_RocketLauncher
    : public AUR_Weapon
{
    GENERATED_BODY()

    AUR_Weap_RocketLauncher(const FObjectInitializer& ObjectInitializer);

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UUR_FireModeCharged* ChargedFireMode;

    UPROPERTY(EditAnywhere, Category = "Weapon|Rockets")
    float RocketsOffset;

    UPROPERTY(EditAnywhere, Category = "Weapon|Rockets")
    float DoubleSpread;

    UPROPERTY(EditAnywhere, Category = "Weapon|Rockets")
    float TripleSpread;

    /** For best results, this value should reflect the average distance from camera to weapon muzzle */
    UPROPERTY(EditAnywhere, Category = "Weapon|Rockets")
    float UseMuzzleDistance;

    virtual void AuthorityShot_Implementation(UUR_FireModeBasic* FireMode, const FSimulatedShotInfo& SimulatedInfo) override;
};
