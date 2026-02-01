// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"

#include "UR_Object_KeyBind.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
UCLASS()
class OPENTOURNAMENT_API UUR_Object_KeyBind : public UObject
{
    GENERATED_BODY()

public:
    UUR_Object_KeyBind();

    UPROPERTY()
    FName Name;

    UPROPERTY()
    FKey Key;
};
