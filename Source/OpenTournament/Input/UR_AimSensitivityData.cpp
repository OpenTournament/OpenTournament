// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_AimSensitivityData.h"

#include "Settings/UR_GamepadSensitivty.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_AimSensitivityData)

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_AimSensitivityData::UUR_AimSensitivityData(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SensitivityMap =
    {
        { EGameGamepadSensitivity::Slow, 0.5f },
        { EGameGamepadSensitivity::SlowPlus, 0.75f },
        { EGameGamepadSensitivity::SlowPlusPlus, 0.9f },
        { EGameGamepadSensitivity::Normal, 1.0f },
        { EGameGamepadSensitivity::NormalPlus, 1.1f },
        { EGameGamepadSensitivity::NormalPlusPlus, 1.25f },
        { EGameGamepadSensitivity::Fast, 1.5f },
        { EGameGamepadSensitivity::FastPlus, 1.75f },
        { EGameGamepadSensitivity::FastPlusPlus, 2.0f },
        { EGameGamepadSensitivity::Insane, 2.5f },
    };
}

const float UUR_AimSensitivityData::SensitivityEnumToFloat(const EGameGamepadSensitivity InSensitivity) const
{
    if (const float* Sens = SensitivityMap.Find(InSensitivity))
    {
        return *Sens;
    }

    return 1.0f;
}
