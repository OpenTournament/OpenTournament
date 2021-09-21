// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "UR_ColorSpectrum.h"	//FOnMouseCapture events, FOnColorChangedEvent
#include "UR_ColorWheel.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class SColorWheel;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Exposes slate ColorWheel as a reusable UMG widget.
 */
UCLASS(BlueprintType, Meta = (DisplayName = "ColorWheel"))
class OPENTOURNAMENT_API UUR_ColorWheel : public UWidget
{
	GENERATED_BODY()

protected:

	TSharedPtr<SColorWheel> MyColorWheel;

	//~ Begin UWidget Interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	//~ End UWidget Interface

protected:

	UPROPERTY(BlueprintReadOnly)
	FLinearColor ColorHSV;

	UPROPERTY(BlueprintReadOnly)
	FLinearColor ColorRGB;

	// Callback for value changes in the color spectrum picker.
	void HandleColorWheelValueChanged(FLinearColor NewValue)
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
		ColorHSV.B = 1.f;
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
