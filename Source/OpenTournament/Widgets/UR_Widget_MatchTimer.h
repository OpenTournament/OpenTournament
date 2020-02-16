// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UR_Widget_MatchTimer.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Base Class for Match Timer Widget
 */
UCLASS()
class OPENTOURNAMENT_API UUR_Widget_MatchTimer : public UUserWidget
{
    GENERATED_BODY()

public:

    UUR_Widget_MatchTimer(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintPure, BlueprintCallable, Category = "MatchTimer")
    FString GetTimeString(const float TimeSeconds) const;

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "MatchTimer")
    int32 RemainingTime;
};
