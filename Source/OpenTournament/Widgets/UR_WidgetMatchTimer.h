// Copyright 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Blueprint/UserWidget.h"
#include "UR_WidgetMatchTimer.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////


/**
 *
 */
UCLASS()
class OPENTOURNAMENT_API UUR_WidgetMatchTimer : public UUserWidget
{
    GENERATED_BODY()

public:

    UUR_WidgetMatchTimer(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintPure, BlueprintCallable, Category = "MatchTimer")
    FString GetTimeString(const float TimeSeconds);

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "MatchTimer")
    int32 RemainingTime;
};
