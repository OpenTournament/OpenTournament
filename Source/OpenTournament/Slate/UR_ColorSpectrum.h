// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Components/Widget.h"

#include "Components/Slider.h"	//FOnMouseCapture events

#include "UR_ColorSpectrum.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class SColorSpectrum;

/////////////////////////////////////////////////////////////////////////////////////////////////

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnColorChangedEvent, FLinearColor, Color);

/**
 * Exposes slate ColorSpectrum as a reusable UMG widget.
 */
UCLASS(BlueprintType, Meta = (DisplayName = "ColorSpectrum"))
class OPENTOURNAMENT_API UUR_ColorSpectrum : public UWidget
{
    GENERATED_BODY()

protected:
    TSharedPtr<SColorSpectrum> MyColorSpectrum;

    //~ Begin UVisual
    virtual void ReleaseSlateResources(bool bReleaseChildren) override;
    //~ End UVisual

    //~ Begin UWidget Interface
    virtual TSharedRef<SWidget> RebuildWidget() override;
    //~ End UWidget Interface

protected:
    UPROPERTY(BlueprintReadOnly)
    FLinearColor ColorHSV;

    UPROPERTY(BlueprintReadOnly)
    FLinearColor ColorRGB;

    // Callback for value changes in the color spectrum picker.
    void HandleColorSpectrumValueChanged(FLinearColor NewValue)
    {
        SetColorHSV(NewValue);
    }

    void HandleOnMouseCaptureBegin()
    {
        OnMouseCaptureBegin.Broadcast();
    }

    void HandleOnMouseCaptureEnd()
    {
        OnMouseCaptureEnd.Broadcast();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////

public:
    UFUNCTION(BlueprintPure)
    FORCEINLINE FLinearColor GetColorRGB() const
    {
        return ColorRGB;
    }

    UFUNCTION(BlueprintPure)
    FORCEINLINE FLinearColor GetColorHSV() const
    {
        return ColorHSV;
    }

    UFUNCTION(BlueprintCallable)
    virtual void SetColorRGB(const FLinearColor& RGB)
    {
        ColorRGB = RGB;
        ColorHSV = ColorRGB.LinearRGBToHSV();

        OnValueChanged.Broadcast(ColorRGB);
    }

    UFUNCTION(BlueprintCallable)
    virtual void SetColorHSV(const FLinearColor& HSV)
    {
        ColorHSV = HSV;
        ColorRGB = ColorHSV.HSVToLinearRGB();

        OnValueChanged.Broadcast(ColorRGB);
    }

    /** Invoked when the mouse is pressed and a capture begins. */
    UPROPERTY(BlueprintAssignable, Category = "Widget Event")
    FOnMouseCaptureBeginEvent OnMouseCaptureBegin;

    /** Invoked when the mouse is released and a capture ends. */
    UPROPERTY(BlueprintAssignable, Category = "Widget Event")
    FOnMouseCaptureEndEvent OnMouseCaptureEnd;

    /** Called when the value is changed. */
    UPROPERTY(BlueprintAssignable, Category = "Widget Event")
    FOnColorChangedEvent OnValueChanged;
};
