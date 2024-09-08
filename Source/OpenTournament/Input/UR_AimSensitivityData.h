// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Engine/DataAsset.h"

#include "UR_AimSensitivityData.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

enum class EGameGamepadSensitivity : uint8;
class UObject;

/////////////////////////////////////////////////////////////////////////////////////////////////

/** Defines a set of gamepad sensitivity to a float value. */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "Game Aim Sensitivity Data", ShortTooltip = "Data asset used to define a map of Gamepad Sensitivty to a float value."))
class OPENTOURNAMENT_API UUR_AimSensitivityData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UUR_AimSensitivityData(const FObjectInitializer& ObjectInitializer);

    const float SensitivityEnumToFloat(const EGameGamepadSensitivity InSensitivity) const;

protected:
    /** Map of SensitivityMap settings to their corresponding float */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EGameGamepadSensitivity, float> SensitivityMap;
};
