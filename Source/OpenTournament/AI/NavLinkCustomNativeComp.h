// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

#pragma once

#include <NavLinkCustomComponent.h>

#include "NavLinkCustomNativeComp.generated.h"


/**
* Native wrapper around NavLinkCustomComponent because its native API sucks.
*/
UCLASS(ClassGroup = AI, Meta = (BlueprintSpawnableComponent))
class OPENTOURNAMENT_API UNavLinkCustomNativeComp : public UNavLinkCustomComponent
{
    GENERATED_BODY()

public:

    void SetRelativeStart(const FVector& Val) { LinkRelativeStart = Val; }
    void SetRelativeEnd(const FVector& Val) { LinkRelativeEnd = Val; }
    void SetDirection(ENavLinkDirection::Type Val) { LinkDirection = Val; }

};
