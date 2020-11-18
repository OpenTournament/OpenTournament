// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UR_Widget_ScoreboardBase.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Scoreboard Base Widget
 */
UCLASS(Abstract)
class OPENTOURNAMENT_API UUR_Widget_ScoreboardBase : public UUserWidget
{
    GENERATED_BODY()

    UUR_Widget_ScoreboardBase(const FObjectInitializer& ObjectInitializer);
};
