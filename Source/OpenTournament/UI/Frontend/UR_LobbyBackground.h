// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Engine/DataAsset.h>

#include "UR_LobbyBackground.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UObject;
class UWorld;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Developer settings / editor cheats
 */
UCLASS(config=EditorPerProjectUserSettings, MinimalAPI)
class UUR_LobbyBackground : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<UWorld> BackgroundLevel;
};
