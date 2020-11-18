// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_ColorWheel.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Colors/SColorWheel.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

TSharedRef<SWidget> UUR_ColorWheel::RebuildWidget()
{
	MyColorWheel = SNew(SColorWheel)
		.SelectedColor_UObject(this, &UUR_ColorWheel::GetColorHSV)
		.OnValueChanged(BIND_UOBJECT_DELEGATE(FOnLinearColorValueChanged, HandleColorWheelValueChanged))
		.OnMouseCaptureBegin(BIND_UOBJECT_DELEGATE(FSimpleDelegate, HandleOnMouseCaptureBegin))
		.OnMouseCaptureEnd(BIND_UOBJECT_DELEGATE(FSimpleDelegate, HandleOnMouseCaptureEnd));

	return MyColorWheel.ToSharedRef();
}
