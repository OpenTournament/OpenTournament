// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "IndicatorLibrary.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

#define UE_API OPENTOURNAMENT_API

class AController;
class UUR_IndicatorManagerComponent;
class UObject;
struct FFrame;

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(MinimalAPI)
class UIndicatorLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UE_API UIndicatorLibrary();

    /**  */
    UFUNCTION(BlueprintCallable, Category = Indicator)
    static UE_API UUR_IndicatorManagerComponent* GetIndicatorManagerComponent(AController* Controller);
};

#undef UE_API
