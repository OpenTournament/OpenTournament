// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_HUDLayout.h"

#include "CommonUIExtensions.h"
#include "NativeGameplayTags.h"
#include "Input/CommonUIInputTypes.h"

#include "UI/UR_ActivatableWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_HUDLayout)

/////////////////////////////////////////////////////////////////////////////////////////////////

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UI_LAYER_MENU, "UI.Layer.Menu");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UI_ACTION_ESCAPE, "UI.Action.Escape");

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_HUDLayout::UUR_HUDLayout(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UUR_HUDLayout::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    RegisterUIActionBinding(FBindUIActionArgs(FUIActionTag::ConvertChecked(TAG_UI_ACTION_ESCAPE), false, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleEscapeAction)));
}

void UUR_HUDLayout::HandleEscapeAction()
{
    if (ensure(!EscapeMenuClass.IsNull()))
    {
        UCommonUIExtensions::PushStreamedContentToLayer_ForPlayer(GetOwningLocalPlayer(), TAG_UI_LAYER_MENU, EscapeMenuClass);
    }
}
