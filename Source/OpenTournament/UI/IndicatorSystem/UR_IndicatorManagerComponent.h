// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Components/ControllerComponent.h"

#include "UR_IndicatorManagerComponent.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

#define UE_API OPENTOURNAMENT_API

class AController;
class UIndicatorDescriptor;
class UObject;
struct FFrame;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @class UUR_IndicatorManagerComponent
 */
UCLASS(MinimalAPI, BlueprintType, Blueprintable)
class UUR_IndicatorManagerComponent : public UControllerComponent
{
    GENERATED_BODY()

public:
    UE_API UUR_IndicatorManagerComponent(const FObjectInitializer& ObjectInitializer);

    static UE_API UUR_IndicatorManagerComponent* GetComponent(AController* Controller);

    UFUNCTION(BlueprintCallable, Category = Indicator)
    UE_API void AddIndicator(UIndicatorDescriptor* IndicatorDescriptor);

    UFUNCTION(BlueprintCallable, Category = Indicator)
    UE_API void RemoveIndicator(UIndicatorDescriptor* IndicatorDescriptor);

    DECLARE_EVENT_OneParam(UUR_IndicatorManagerComponent, FIndicatorEvent, UIndicatorDescriptor* Descriptor)

    FIndicatorEvent OnIndicatorAdded;
    FIndicatorEvent OnIndicatorRemoved;

    const TArray<UIndicatorDescriptor*>& GetIndicators() const
    {
        return Indicators;
    }

private:
    UPROPERTY()
    TArray<TObjectPtr<UIndicatorDescriptor>> Indicators;
};

#undef UE_API
