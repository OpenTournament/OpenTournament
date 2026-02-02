// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Engine/DeveloperSettings.h"

#include <GameplayTagContainer.h>

#include "UR_ContextEffectsSettings.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

#define UE_API OPENTOURNAMENT_API

enum EPhysicalSurface : int;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
UCLASS(MinimalAPI, Config = Game, defaultconfig, meta = (DisplayName = "GameContextEffects"))
class UUR_ContextEffectsSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UPROPERTY(Config, EditAnywhere)
    TMap<TEnumAsByte<EPhysicalSurface>, FGameplayTag> SurfaceTypeToContextMap;
};

#undef UE_API
