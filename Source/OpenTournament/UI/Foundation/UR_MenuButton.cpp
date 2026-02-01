// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_MenuButton.h"

#include "CommonTextBlock.h"
#include "Animation/UMGSequencePlayer.h"
#include "Animation/WidgetAnimation.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Border.h"
#include "Components/HorizontalBoxSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Image.h"

#include "UR_ActionWidget.h"
#include "Components/WidgetSwitcherSlot.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_MenuButton)

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_MenuButton::UUR_MenuButton()
    : Super()
    , TextCase(ETextTransformPolicy::ToUpper)
    , TextPadding(FMargin(32.f, 0.f))
    , bUseImageOverlays(true)
    , bUseScaleChangeSpacers(true)
    , bUseIconOverride(false)
    , InputAlignmentX(EHorizontalAlignment::HAlign_Left)
    , InputAlignmentY(EVerticalAlignment::VAlign_Center)
    , InputPadding(FMargin(-12.f, 0.f, 0.f, 0.f))
    , HoverAnimationParameter(FName("Hover_Animate"))
    , PressedAnimationParameter(FName("Pressed_Animate"))
{}

void UUR_MenuButton::NativePreConstruct()
{
    Super::NativePreConstruct();

    UpdateTextStyle();
    ResetMaterials();
}

void UUR_MenuButton::NativeOnHovered()
{
    Super::NativeOnHovered();

    if (IsValid(AnimateHover))
    {
        auto StartTime = GetAnimationCurrentTime(AnimateHover);
        PlayAnimation(AnimateHover, StartTime, 1, EUMGSequencePlayMode::Forward, 1.f);
    }

    auto SoundToPlay = bUseIconOverride ? HoverIconSound : HoverSound;
    if (SoundToPlay)
    {
        UGameplayStatics::PlaySound2D(GetOwningPlayer(), SoundToPlay);
    }
}

void UUR_MenuButton::NativeOnUnhovered()
{
    Super::NativeOnUnhovered();

    if (IsValid(AnimateHover))
    {
        auto StartTime = GetAnimationCurrentTime(AnimateHover);
        PlayAnimation(AnimateHover, StartTime, 1, EUMGSequencePlayMode::Reverse, 1.f);
    }
}

void UUR_MenuButton::NativeOnSelected(bool bBroadcast)
{
    Super::NativeOnSelected(bBroadcast);

    if (GetIsEnabled())
    {
        if (IsValid(AnimatePress))
        {
            auto StartTime = GetAnimationCurrentTime(AnimatePress);
            PlayAnimation(AnimatePress, StartTime, 1, EUMGSequencePlayMode::Forward, 1.f);
        }
    }
}

void UUR_MenuButton::NativeOnDeselected(bool bBroadcast)
{
    Super::NativeOnDeselected(bBroadcast);

    if (GetIsEnabled())
    {
        if (IsValid(AnimatePress))
        {
            auto StartTime = GetAnimationCurrentTime(AnimatePress);
            PlayAnimation(AnimatePress, StartTime, 1, EUMGSequencePlayMode::Reverse, 1.f);
        }
    }
}

void UUR_MenuButton::NativeOnEnabled()
{
    Super::NativeOnEnabled();

    SetMaterialsEnabled();
    ResetMaterials();
}

void UUR_MenuButton::NativeOnDisabled()
{
    Super::NativeOnDisabled();

    SetMaterialsEnabled();
}

void UUR_MenuButton::NativeOnClicked()
{
    Super::NativeOnClicked();

    if (GetIsEnabled())
    {
        if (IsValid(AnimatePress))
        {
            const auto StartTime = GetAnimationCurrentTime(AnimatePress);
            auto Animation = PlayAnimation(AnimatePress, StartTime, 1, EUMGSequencePlayMode::Reverse, 1.f);
            Animation.GetAnimationState()->GetOnWidgetAnimationFinished().AddUObject(this, &ThisClass::OnPressedAnimationFinished);
        }

        auto SoundToPlay = bUseIconOverride ? HoverIconSound : HoverSound;
        if (SoundToPlay)
        {
            UGameplayStatics::PlaySound2D(GetOwningPlayer(), SoundToPlay);
        }
    }
}

void UUR_MenuButton::UpdateInputActionWidget()
{
    Super::UpdateInputActionWidget();

    UpdateButtonStyle();
    RefreshButtonText();
}

void UUR_MenuButton::OnInputMethodChanged(ECommonInputType CurrentInputType)
{
    Super::OnInputMethodChanged(CurrentInputType);

    UpdateButtonStyle();
}

void UUR_MenuButton::UpdateButtonText_Implementation(const FText& InText)
{
    if (IsValid(ButtonTextBlock))
    {
        ButtonTextBlock->SetText(InText);
    }
}

void UUR_MenuButton::UpdateButtonStyle_Implementation()
{
    if (IsValid(Border))
    {
        Border->SetBrush(BrushButtonBorder);

        if (auto* BoxSlot = UWidgetLayoutLibrary::SlotAsHorizontalBoxSlot(Border))
        {
            BoxSlot->SetPadding(HitTestPadding);
        }
    }

    if (IsValid(InputActionWidget))
    {
        if (auto OverlaySlot = UWidgetLayoutLibrary::SlotAsOverlaySlot(InputActionWidget))
        {
            OverlaySlot->SetPadding(InputPadding);
            OverlaySlot->SetHorizontalAlignment(InputAlignmentX);
            OverlaySlot->SetVerticalAlignment(InputAlignmentY);
        }
    }

    if (bUseIconOverride)
    {
        if (IsValid(TextIconSwitcher))
        {
            TextIconSwitcher->SetActiveWidget(OverlayIcon);
        }

        if (IsValid(Icon))
        {
            Icon->SetBrush(BrushIcon);
        }
    }
    else
    {
        if (IsValid(TextIconSwitcher))
        {
            TextIconSwitcher->SetActiveWidget(OverlayText);
        }

        if (IsValid(OverlayText))
        {
            if (auto SwitcherSlot = UWidgetLayoutLibrary::SlotAsWidgetSwitcherSlot(OverlayText))
            {
                SwitcherSlot->SetPadding(TextPadding);
            }
        }
    }
}

void UUR_MenuButton::UpdateTextStyle_Implementation()
{
    if (IsValid(ButtonTextBlock))
    {
        ButtonTextBlock->SetFont(Font);
        ButtonTextBlock->SetTextTransformPolicy(TextCase);
    }
}

void UUR_MenuButton::SetMaterialsEnabled()
{
    float ScalarValue = GetIsEnabled() ? 0.f : 1.f;
    if (IsValid(Border))
    {
        if (auto BorderMID = Border->GetDynamicMaterial())
        {
            BorderMID->SetScalarParameterValue(DisabledParameter, ScalarValue);
        }
    }

    if (IsValid(ButtonTextBlock))
    {
        if (auto TextMID = ButtonTextBlock->GetDynamicFontMaterial())
        {
            TextMID->SetScalarParameterValue(DisabledParameter, ScalarValue);
        }
    }
}

void UUR_MenuButton::OnPressedAnimationFinished(FWidgetAnimationState& Event)
{
    if (GetIsEnabled())
    {
        auto SoundToPlay = bUseIconOverride ? HoverIconSound : HoverSound;
        if (SoundToPlay)
        {
            UGameplayStatics::PlaySound2D(GetOwningPlayer(), SoundToPlay);
        }
    }
    else
    {
        PlayAnimationReverse(AnimatePress);

        if (IsValid(DeactivatedSound))
        {
            UGameplayStatics::PlaySound2D(GetOwningPlayer(), DeactivatedSound);
        }
    }
}

void UUR_MenuButton::ResetMaterials_Implementation()
{
    if (IsValid(Border))
    {
        if (auto BorderMID = Border->GetDynamicMaterial())
        {
            BorderMID->SetScalarParameterValue(HoverAnimationParameter, 0.0f);
            BorderMID->SetScalarParameterValue(PressedAnimationParameter, 0.0f);
        }
    }

    if (IsValid(ButtonTextBlock))
    {
        if (auto TextMID = ButtonTextBlock->GetDynamicFontMaterial())
        {
            TextMID->SetScalarParameterValue(HoverAnimationParameter, 0.0f);
            TextMID->SetScalarParameterValue(PressedAnimationParameter, 0.0f);
        }
    }
}
