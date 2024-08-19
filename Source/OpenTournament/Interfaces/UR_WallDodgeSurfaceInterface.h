// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "UObject/Interface.h"

#include "UR_WallDodgeSurfaceInterface.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Interface for WallDodge Surface Actors
 */
UINTERFACE(BlueprintType)
class OPENTOURNAMENT_API UUR_WallDodgeSurfaceInterface
    : public UInterface
{
    GENERATED_BODY()
};

class IUR_WallDodgeSurfaceInterface
{
    GENERATED_BODY()

public:
    /**
    * Is WallDodge Permitted?
    */
    UFUNCTION(BlueprintNativeEvent, Category = "WallDodgeSurfaceInterface")
    bool IsWallDodgePermitted() const;
};
