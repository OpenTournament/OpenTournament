// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UR_ButtonBase.h"

#include "UR_MenuButton.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UOverlay;
class UImage;
class UWidgetSwitcher;
class UUR_ActionWidget;
class UBorder;
class UCommonTextBlock;
class UObject;
struct FFrame;

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(Abstract, BlueprintType, Blueprintable)
class UUR_MenuButton : public UUR_ButtonBase
{
    GENERATED_BODY()

public:
    UUR_MenuButton();

    // UUserWidget interface
    virtual void NativePreConstruct() override;
    virtual void NativeOnHovered() override;
    virtual void NativeOnUnhovered() override;
    virtual void NativeOnSelected(bool bBroadcast) override;
    virtual void NativeOnDeselected(bool bBroadcast) override;
    virtual void NativeOnEnabled() override;
    virtual void NativeOnDisabled() override;
    virtual void NativeOnClicked() override;
    // End of UUserWidget interface

    // UCommonButtonBase interface
    virtual void UpdateInputActionWidget() override;
    virtual void OnInputMethodChanged(ECommonInputType CurrentInputType) override;
    // End of UCommonButtonBase interface

    // UUR_ButtonBase interface
    void UpdateButtonText_Implementation(const FText& InText);
    void UpdateButtonStyle_Implementation();
    // End of UUR_ButtonBase interface


    UFUNCTION(BlueprintNativeEvent)
    void UpdateTextStyle();

    void SetMaterialsEnabled();

    UFUNCTION(BlueprintNativeEvent)
    void ResetMaterials();

    // Callback for Pressed Animation Finished Play
    virtual void OnPressedAnimationFinished(FWidgetAnimationState& Event);

    UPROPERTY(EditAnywhere)
    FSlateFontInfo Font;

    UPROPERTY(EditAnywhere)
    ETextTransformPolicy TextCase;

    UPROPERTY(EditAnywhere)
    FMargin TextPadding;

    UPROPERTY(EditAnywhere)
    FSlateBrush BrushButtonBorder;

    UPROPERTY(EditAnywhere)
    FSlateBrush BrushIcon;

    UPROPERTY(EditAnywhere)
    bool bUseImageOverlays;

    UPROPERTY(EditAnywhere)
    bool bUseScaleChangeSpacers;

    UPROPERTY(EditAnywhere)
    bool bUseIconOverride;

    UPROPERTY(EditAnywhere)
    TEnumAsByte<EHorizontalAlignment> InputAlignmentX;

    UPROPERTY(EditAnywhere)
    TEnumAsByte<EVerticalAlignment> InputAlignmentY;

    UPROPERTY(EditAnywhere)
    FMargin HitTestPadding;

    UPROPERTY(EditAnywhere)
    FMargin InputPadding;

    ///

    UPROPERTY(EditAnywhere, Category="Sound")
    TObjectPtr<USoundBase> HoverSound;

    UPROPERTY(EditAnywhere, Category="Sound")
    TObjectPtr<USoundBase> HoverIconSound;

    UPROPERTY(EditAnywhere, Category="Sound")
    TObjectPtr<USoundBase> DeactivatedSound;

    ///


    UPROPERTY(EditAnywhere, Category="Material")
    FName DisabledParameter;

    UPROPERTY(EditAnywhere, Category="Material")
    FName HoverAnimationParameter;

    UPROPERTY(EditAnywhere, Category="Material")
    FName PressedAnimationParameter;

    ////

    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UCommonTextBlock> ButtonTextBlock;

    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UBorder> Border;

    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UWidgetSwitcher> TextIconSwitcher;

    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UOverlay> OverlayIcon;

    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UOverlay> OverlayText;

    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UImage> Icon;

    ////

    UPROPERTY(Transient, meta=(BindWidgetAnim))
    TObjectPtr<UWidgetAnimation> AnimateHover;

    UPROPERTY(Transient, meta=(BindWidgetAnim))
    TObjectPtr<UWidgetAnimation> AnimatePress;

private:
};
