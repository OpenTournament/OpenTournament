// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_ColorSpectrum.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Colors/SColorSpectrum.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

TSharedRef<SWidget> UUR_ColorSpectrum::RebuildWidget()
{
	MyColorSpectrum = SNew(SColorSpectrum)
		.SelectedColor_UObject(this, &UUR_ColorSpectrum::GetColorHSV)
		.OnValueChanged(BIND_UOBJECT_DELEGATE(FOnLinearColorValueChanged, HandleColorSpectrumValueChanged))
		.OnMouseCaptureBegin(BIND_UOBJECT_DELEGATE(FSimpleDelegate, HandleOnMouseCaptureBegin))
		.OnMouseCaptureEnd(BIND_UOBJECT_DELEGATE(FSimpleDelegate, HandleOnMouseCaptureEnd));

	return MyColorSpectrum.ToSharedRef();
}
