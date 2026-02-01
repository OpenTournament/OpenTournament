// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <GameFeatureAction.h>

#include "UR_GFA_ApplyFrontendPerfSettings.generated.h"

class UObject;
struct FGameFeatureActivatingContext;
struct FGameFeatureDeactivatingContext;

//////////////////////////////////////////////////////////////////////
// UUR_GFA_ApplyFrontendPerfSettings

/**
 * GameFeatureAction responsible for telling the user settings to apply frontend/menu specific performance settings
 */
UCLASS(MinimalAPI, meta = (DisplayName = "Use Frontend Perf Settings"))
class UUR_GFA_ApplyFrontendPerfSettings final : public UGameFeatureAction
{
    GENERATED_BODY()

public:
    //~UGameFeatureAction interface
    virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
    virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
    //~End of UGameFeatureAction interface

private:
    static int32 ApplicationCounter;
};
