// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UR_WidgetMatchTimer.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Base Class for Match Timer Widget
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
