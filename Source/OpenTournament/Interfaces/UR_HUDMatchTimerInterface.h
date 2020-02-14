// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "CoreMinimal.h"
#include "Interface.h"

#include "UR_HUDMatchTimerInterface.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Interface to Open or Close MatchTimer
 */
UINTERFACE(BlueprintType)
class OPENTOURNAMENT_API UUR_HUDMatchTimerInterface : public UInterface
{
    GENERATED_BODY()
};

class IUR_HUDMatchTimerInterface
{
    GENERATED_BODY()

public:

    /**
    * Open Match Timer
    */
    UFUNCTION()
    virtual void OpenMatchTimer() PURE_VIRTUAL(IUR_HUDMatchTimerInterface::OpenMatchTimer,);

    /**
    * Close Match Timer
    */
    UFUNCTION()
    virtual void CloseMatchTimer() PURE_VIRTUAL(IUR_HUDMatchTimerInterface::CloseMatchTimer,);
};
