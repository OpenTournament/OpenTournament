// Copyright 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UR_UserWidgetMenu.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Delegates

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTransitionCompleteSignature);

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Menu UserWidget
 */
UClass(BlueprintType)
class OPENTOURNAMENT_API UUR_UserWidgetMenu : public UUserWidget
{
    GENERATED_BODY()

    /**
    * On Widget Added to Menu Manager Stack
    */
    UFUNCTION(BlueprintNativeEvent, Category = "Menu")
    void OnAdded();

    /**
    * On Widget Added to Menu Manager Stack and Completed any Additional Steps
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Menu")
    FOnTransitionComplete OnAddedComplete;


    /**
    * On Widget Removed from Menu Manager Stack
    */
    UFUNCTION(BlueprintNativeEvent, Category = "Menu")
    void OnRemoved();

    /**
    * On Widget Removed from Menu Manager Stack and Completed any Additional Steps
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Menu")
    FOnTransitionComplete OnRemovedComplete;

    /**
    * On Widget Revealed from Menu Manager Stack
    */
    UFUNCTION(BlueprintNativeEvent, Category = "Menu")
    void OnRevealed();

    /**
    * On Widget Revealed from Menu Manager Stack and Completed any Additional Steps
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Menu")
    FOnTransitionComplete OnRevealedComplete;

    /**
    * On Widget Covered from Menu Manager Stack
    */
    UFUNCTION(BlueprintNativeEvent, Category = "Menu")
    void OnCovered();

    /**
    * On Widget Covered from Menu Manager Stack and Completed any Additional Steps
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Menu")
    FOnTransitionComplete OnCoveredComplete;
};