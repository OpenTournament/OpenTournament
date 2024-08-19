// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_ActivatableWidget.h"

#include "Editor/WidgetCompilerLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_ActivatableWidget)

/////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "OTUI"

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_ActivatableWidget::UUR_ActivatableWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

TOptional<FUIInputConfig> UUR_ActivatableWidget::GetDesiredInputConfig() const
{
    switch (InputConfig)
    {
        case EGameWidgetInputMode::GameAndMenu:
        {
            return FUIInputConfig(ECommonInputMode::All, GameMouseCaptureMode);
        }
        case EGameWidgetInputMode::Game:
        {
            return FUIInputConfig(ECommonInputMode::Game, GameMouseCaptureMode);
        }
        case EGameWidgetInputMode::Menu:
        {
            return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
        }
        case EGameWidgetInputMode::Default:
        default:
        {
            return TOptional<FUIInputConfig>();
        }
    }
}

#if WITH_EDITOR

void UUR_ActivatableWidget::ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, class IWidgetCompilerLog& CompileLog) const
{
    Super::ValidateCompiledWidgetTree(BlueprintWidgetTree, CompileLog);

    if (!GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UUR_ActivatableWidget, BP_GetDesiredFocusTarget)))
    {
        if (GetParentNativeClass(GetClass()) == UUR_ActivatableWidget::StaticClass())
        {
            CompileLog.Warning(LOCTEXT("ValidateGetDesiredFocusTarget_Warning", "GetDesiredFocusTarget wasn't implemented, you're going to have trouble using gamepads on this screen."));
        }
        else
        {
            //TODO - Note for now, because we can't guarantee it isn't implemented in a native subclass of this one.
            CompileLog.Note
            (LOCTEXT
                ("ValidateGetDesiredFocusTarget_Note",
                    "GetDesiredFocusTarget wasn't implemented, you're going to have trouble using gamepads on this screen.  If it was implemented in the native base class you can ignore this message."));
        }
    }
}

#endif

#undef LOCTEXT_NAMESPACE
