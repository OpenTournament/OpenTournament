// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_ConfirmationScreen.h"

#include <CommonBorder.h>
#include <CommonRichTextBlock.h>
#include <CommonTextBlock.h>
#include <ICommonInputModule.h>
#include <Components/DynamicEntryBox.h>

#include "UR_ButtonBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_ConfirmationScreen)

#if WITH_EDITOR
#include "CommonInputSettings.h"
#include "Editor/WidgetCompilerLog.h"
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_ConfirmationScreen::SetupDialog(UCommonGameDialogDescriptor* Descriptor, FCommonMessagingResultDelegate ResultCallback)
{
    Super::SetupDialog(Descriptor, ResultCallback);

    Text_Title->SetText(Descriptor->Header);
    RichText_Description->SetText(Descriptor->Body);

    EntryBox_Buttons->Reset<UUR_ButtonBase>([](UUR_ButtonBase& Button)
    {
        Button.OnClicked().Clear();
    });

    for (const FConfirmationDialogAction& Action : Descriptor->ButtonActions)
    {
        FDataTableRowHandle ActionRow;

        switch (Action.Result)
        {
            case ECommonMessagingResult::Confirmed:
                ActionRow = ICommonInputModule::GetSettings().GetDefaultClickAction();
                break;
            case ECommonMessagingResult::Declined:
                ActionRow = ICommonInputModule::GetSettings().GetDefaultBackAction();
                break;
            case ECommonMessagingResult::Cancelled:
                ActionRow = CancelAction;
                break;
            default:
                ensure(false);
                continue;
        }

        UUR_ButtonBase* Button = EntryBox_Buttons->CreateEntry<UUR_ButtonBase>();
        Button->SetTriggeringInputAction(ActionRow);
        Button->OnClicked().AddUObject(this, &ThisClass::CloseConfirmationWindow, Action.Result);
        Button->SetButtonText(Action.OptionalDisplayText);
    }

    OnResultCallback = ResultCallback;
}

void UUR_ConfirmationScreen::KillDialog()
{
    Super::KillDialog();
}

void UUR_ConfirmationScreen::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    Border_TapToCloseZone->OnMouseButtonDownEvent.BindDynamic(this, &UUR_ConfirmationScreen::HandleTapToCloseZoneMouseButtonDown);
}

void UUR_ConfirmationScreen::CloseConfirmationWindow(ECommonMessagingResult Result)
{
    DeactivateWidget();
    OnResultCallback.ExecuteIfBound(Result);
}

FEventReply UUR_ConfirmationScreen::HandleTapToCloseZoneMouseButtonDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
    FEventReply Reply;
    Reply.NativeReply = FReply::Unhandled();

    if (MouseEvent.IsTouchEvent() || MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        CloseConfirmationWindow(ECommonMessagingResult::Declined);
        Reply.NativeReply = FReply::Handled();
    }

    return Reply;
}

#if WITH_EDITOR
void UUR_ConfirmationScreen::ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const
{
    if (CancelAction.IsNull())
    {
        CompileLog.Error(FText::Format(FText::FromString(TEXT("{0} has unset property: CancelAction.")), FText::FromString(GetName())));
    }
}
#endif
