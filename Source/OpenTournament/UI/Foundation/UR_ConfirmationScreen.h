// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Messaging/CommonGameDialog.h>

#include <Engine/DataTable.h>
#include <Messaging/CommonMessagingSubsystem.h>

#include "UR_ConfirmationScreen.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class IWidgetCompilerLog;

class UCommonTextBlock;
class UCommonRichTextBlock;
class UDynamicEntryBox;
class UCommonBorder;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class UUR_ConfirmationScreen : public UCommonGameDialog
{
    GENERATED_BODY()

public:
    virtual void SetupDialog(UCommonGameDialogDescriptor* Descriptor, FCommonMessagingResultDelegate ResultCallback) override;
    virtual void KillDialog() override;

protected:
    virtual void NativeOnInitialized() override;
    virtual void CloseConfirmationWindow(ECommonMessagingResult Result);

#if WITH_EDITOR
    virtual void ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const override;
#endif

private:
    UFUNCTION()
    FEventReply HandleTapToCloseZoneMouseButtonDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent);

    FCommonMessagingResultDelegate OnResultCallback;

private:
    UPROPERTY(Meta = (BindWidget))
    TObjectPtr<UCommonTextBlock> Text_Title;

    UPROPERTY(Meta = (BindWidget))
    TObjectPtr<UCommonRichTextBlock> RichText_Description;

    UPROPERTY(Meta = (BindWidget))
    TObjectPtr<UDynamicEntryBox> EntryBox_Buttons;

    UPROPERTY(Meta = (BindWidget))
    TObjectPtr<UCommonBorder> Border_TapToCloseZone;

    UPROPERTY(EditDefaultsOnly, meta = (RowType = "/Script/CommonUI.CommonInputActionDataBase"))
    FDataTableRowHandle CancelAction;
};
