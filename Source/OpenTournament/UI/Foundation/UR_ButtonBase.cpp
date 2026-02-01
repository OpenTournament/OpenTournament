// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_ButtonBase.h"

#include <CommonActionWidget.h>

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_ButtonBase)

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_ButtonBase::NativePreConstruct()
{
    Super::NativePreConstruct();

    UpdateButtonStyle();
    RefreshButtonText();
}

void UUR_ButtonBase::UpdateInputActionWidget()
{
    Super::UpdateInputActionWidget();

    UpdateButtonStyle();
    RefreshButtonText();
}

void UUR_ButtonBase::SetButtonText(const FText& InText)
{
    bOverride_ButtonText = InText.IsEmpty();
    ButtonText = InText;
    RefreshButtonText();
}

void UUR_ButtonBase::RefreshButtonText()
{
    if (bOverride_ButtonText || ButtonText.IsEmpty())
    {
        if (InputActionWidget)
        {
            const FText ActionDisplayText = InputActionWidget->GetDisplayText();
            if (!ActionDisplayText.IsEmpty())
            {
                UpdateButtonText(ActionDisplayText);
                return;
            }
        }
    }

    UpdateButtonText(ButtonText);
}

void UUR_ButtonBase::OnInputMethodChanged(ECommonInputType CurrentInputType)
{
    Super::OnInputMethodChanged(CurrentInputType);

    UpdateButtonStyle();
}
