// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "Templates/SubclassOf.h"

#include "UObject/WeakObjectPtr.h"
#include "UR_LoadingScreenSubsystem.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UObject;
class UUserWidget;
struct FFrame;

/////////////////////////////////////////////////////////////////////////////////////////////////

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLoadingScreenWidgetChangedDelegate, TSubclassOf<UUserWidget>, NewWidgetClass);

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Tracks/stores the current loading screen configuration in a place
 * that persists across map transitions
 */
UCLASS()
class OPENTOURNAMENT_API UUR_LoadingScreenSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UUR_LoadingScreenSubsystem();

    // Sets the loading screen widget class to display inside of the loading screen widget host
    UFUNCTION(BlueprintCallable)
    void SetLoadingScreenContentWidget(TSubclassOf<UUserWidget> NewWidgetClass);

    // Returns the last set loading screen widget class to display inside of the loading screen widget host
    UFUNCTION(BlueprintPure)
    TSubclassOf<UUserWidget> GetLoadingScreenContentWidget() const;

private:
    UPROPERTY(BlueprintAssignable, meta=(AllowPrivateAccess))
    FLoadingScreenWidgetChangedDelegate OnLoadingScreenWidgetChanged;

    UPROPERTY()
    TSubclassOf<UUserWidget> LoadingScreenWidgetClass;
};
