// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UR_ActivatableWidget.h"

#include "UR_HUDLayout.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UCommonActivatableWidget;
class UObject;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * UUR_HUDLayout
 *
 *	Widget used to lay out the player's HUD (typically specified by an Add Widgets action in the experience)
 */
UCLASS(Abstract, BlueprintType, Blueprintable, Meta = (DisplayName = "UR_ HUD Layout", Category = "OT|HUD"))
class UUR_HUDLayout : public UUR_ActivatableWidget
{
    GENERATED_BODY()

public:
    UUR_HUDLayout(const FObjectInitializer& ObjectInitializer);

    virtual void NativeOnInitialized() override;

protected:
    void HandleEscapeAction();

    UPROPERTY(EditDefaultsOnly)
    TSoftClassPtr<UCommonActivatableWidget> EscapeMenuClass;
};
