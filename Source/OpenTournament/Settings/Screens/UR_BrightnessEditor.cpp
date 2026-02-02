// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_BrightnessEditor.h"

#include "CommonButtonBase.h"
#include "CommonRichTextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "GameSettingValueScalar.h"
#include "Settings/UR_SettingsLocal.h"
#include "Widgets/Layout/SSafeZone.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_BrightnessEditor)

/////////////////////////////////////////////////////////////////////////////////////////////////

struct FGeometry;

/////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "OT"

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace BrightnessEditor
{
    const float JoystickDeadZone = 0.2f;
    const float SafeZoneChangeSpeed = 0.1f;
}

UUR_BrightnessEditor::UUR_BrightnessEditor(const FObjectInitializer& Initializer)
    : Super(Initializer)
{
    UUserWidget::SetVisibility(ESlateVisibility::Visible);
    SetIsFocusable(true);
}

void UUR_BrightnessEditor::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    Switcher_SafeZoneMessage->SetActiveWidget(RichText_Default);
}

void UUR_BrightnessEditor::NativeOnActivated()
{
    Super::NativeOnActivated();

    SSafeZone::SetGlobalSafeZoneScale(UUR_SettingsLocal::Get()->GetSafeZone());

    Button_Done->OnClicked().AddUObject(this, &ThisClass::HandleDoneClicked);

    Button_Back->SetVisibility((bCanCancel) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    if (bCanCancel)
    {
        Button_Back->OnClicked().AddUObject(this, &ThisClass::HandleBackClicked);
    }
}

bool UUR_BrightnessEditor::ExecuteActionForSetting_Implementation(FGameplayTag ActionTag, UGameSetting* InSetting)
{
    TArray<UGameSetting*> ChildSettings = InSetting ? InSetting->GetChildSettings() : TArray<UGameSetting*>();
    if (ChildSettings.Num() > 0 && ChildSettings[0])
    {
        ValueSetting = Cast<UGameSettingValueScalar>(ChildSettings[0]);
    }

    return true;
}

FReply UUR_BrightnessEditor::NativeOnAnalogValueChanged(const FGeometry& InGeometry, const FAnalogInputEvent& InAnalogEvent)
{
    if (InAnalogEvent.GetKey() == EKeys::Gamepad_LeftY && FMath::Abs(InAnalogEvent.GetAnalogValue()) >= BrightnessEditor::JoystickDeadZone)
    {
        const float SafeZoneMultiplier = FMath::Clamp(SSafeZone::GetGlobalSafeZoneScale().Get(1.0f) + InAnalogEvent.GetAnalogValue() * BrightnessEditor::SafeZoneChangeSpeed, 0.0f, 1.0f);
        SSafeZone::SetGlobalSafeZoneScale(SafeZoneMultiplier >= 0 ? SafeZoneMultiplier : 0);

        return FReply::Handled();
    }
    return Super::NativeOnAnalogValueChanged(InGeometry, InAnalogEvent);
}

FReply UUR_BrightnessEditor::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    const float SafeZoneMultiplier = FMath::Clamp(SSafeZone::GetGlobalSafeZoneScale().Get(1.0f) + InMouseEvent.GetWheelDelta() * BrightnessEditor::SafeZoneChangeSpeed, 0.0f, 1.0f);
    SSafeZone::SetGlobalSafeZoneScale(SafeZoneMultiplier >= 0 ? SafeZoneMultiplier : 0);

    return FReply::Handled();
}

void UUR_BrightnessEditor::HandleInputModeChanged(ECommonInputType InInputType)
{
    const FText KeyName = InInputType == ECommonInputType::Gamepad ? LOCTEXT("SafeZone_KeyToPress_Gamepad", "Left Stick") : LOCTEXT("SafeZone_KeyToPress_Mouse", "Mouse Wheel");
    RichText_Default->SetText(FText::Format(LOCTEXT("BrightnessAdjustInstructions", "Use <text color=\"FFFFFFFF\" fontface=\"black\">{0}</> to adjust the brightness"), KeyName));
}

void UUR_BrightnessEditor::HandleBackClicked()
{
    DeactivateWidget();
    SSafeZone::SetGlobalSafeZoneScale(UUR_SettingsLocal::Get()->GetSafeZone());
}

void UUR_BrightnessEditor::HandleDoneClicked()
{
    if (ValueSetting.IsValid())
    {
        ValueSetting.Get()->SetValue(SSafeZone::GetGlobalSafeZoneScale().Get(1.0f));
    }
    else
    {
        UUR_SettingsLocal::Get()->SetSafeZone(SSafeZone::GetGlobalSafeZoneScale().Get(1.0f));
    }
    OnSafeZoneSet.Broadcast();
    DeactivateWidget();
}

#undef LOCTEXT_NAMESPACE
