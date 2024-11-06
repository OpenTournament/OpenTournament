// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_ColorWheel.h"

#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Colors/SColorWheel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_ColorWheel)

/////////////////////////////////////////////////////////////////////////////////////////////////

TSharedRef<SWidget> UUR_ColorWheel::RebuildWidget()
{
    MyColorWheel = SNew(SColorWheel)
        .SelectedColor_UObject(this, &ThisClass::GetColorHSV)
        .OnValueChanged(BIND_UOBJECT_DELEGATE(FOnLinearColorValueChanged, HandleColorWheelValueChanged))
        .OnMouseCaptureBegin(BIND_UOBJECT_DELEGATE(FSimpleDelegate, HandleOnMouseCaptureBegin))
        .OnMouseCaptureEnd(BIND_UOBJECT_DELEGATE(FSimpleDelegate, HandleOnMouseCaptureEnd));

    return MyColorWheel.ToSharedRef();
}
