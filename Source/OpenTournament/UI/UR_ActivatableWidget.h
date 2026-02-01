// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CommonActivatableWidget.h"

#include "UR_ActivatableWidget.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

struct FUIInputConfig;

UENUM(BlueprintType)
enum class EGameWidgetInputMode : uint8
{
    Default,
    GameAndMenu,
    Game,
    Menu
};

/////////////////////////////////////////////////////////////////////////////////////////////////

// An activatable widget that automatically drives the desired input config when activated
UCLASS(Abstract, Blueprintable)
class UUR_ActivatableWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    UUR_ActivatableWidget(const FObjectInitializer& ObjectInitializer);

public:
    //~UCommonActivatableWidget interface
    virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

    //~End of UCommonActivatableWidget interface

#if WITH_EDITOR
    virtual void ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, class IWidgetCompilerLog& CompileLog) const override;
#endif

protected:
    /** The desired input mode to use while this UI is activated, for example do you want key presses to still reach the game/player controller? */
    UPROPERTY(EditDefaultsOnly, Category = Input)
    EGameWidgetInputMode InputConfig = EGameWidgetInputMode::Default;

    /** The desired mouse behavior when the game gets input. */
    UPROPERTY(EditDefaultsOnly, Category = Input)
    EMouseCaptureMode GameMouseCaptureMode = EMouseCaptureMode::CapturePermanently;
};
