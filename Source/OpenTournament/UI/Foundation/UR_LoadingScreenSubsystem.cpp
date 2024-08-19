// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UI/Foundation/UR_LoadingScreenSubsystem.h"

#include "Blueprint/UserWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_LoadingScreenSubsystem)

/////////////////////////////////////////////////////////////////////////////////////////////////

class UUserWidget;

//////////////////////////////////////////////////////////////////////
// UUR_LoadingScreenSubsystem

UUR_LoadingScreenSubsystem::UUR_LoadingScreenSubsystem()
{
}

void UUR_LoadingScreenSubsystem::SetLoadingScreenContentWidget(TSubclassOf<UUserWidget> NewWidgetClass)
{
    if (LoadingScreenWidgetClass != NewWidgetClass)
    {
        LoadingScreenWidgetClass = NewWidgetClass;

        OnLoadingScreenWidgetChanged.Broadcast(LoadingScreenWidgetClass);
    }
}

TSubclassOf<UUserWidget> UUR_LoadingScreenSubsystem::GetLoadingScreenContentWidget() const
{
    return LoadingScreenWidgetClass;
}
