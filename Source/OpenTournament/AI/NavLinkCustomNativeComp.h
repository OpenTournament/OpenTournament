// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <NavLinkCustomComponent.h>

#include "NavLinkCustomNativeComp.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
* Native wrapper around NavLinkCustomComponent because its native API sucks.
*/
UCLASS(ClassGroup = AI, Meta = (BlueprintSpawnableComponent))
class OPENTOURNAMENT_API UNavLinkCustomNativeComp : public UNavLinkCustomComponent
{
    GENERATED_BODY()

public:
    void SetRelativeStart(const FVector& InVector)
    {
        LinkRelativeStart = InVector;
    }

    void SetRelativeEnd(const FVector& InVector)
    {
        LinkRelativeEnd = InVector;
    }

    void SetDirection(const ENavLinkDirection::Type InDirection)
    {
        LinkDirection = InDirection;
    }
};
