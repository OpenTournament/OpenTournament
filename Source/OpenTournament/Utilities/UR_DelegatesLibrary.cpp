// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_DelegatesLibrary.h"

#include "Engine.h"
#include "UnrealClient.h"

UUR_DelegatesLibrary::UUR_DelegatesLibrary()
{
    GEngine->GameViewport->Viewport->ViewportResizedEvent.AddUObject(this, &UUR_DelegatesLibrary::InternalOnViewportResized);
}

void UUR_DelegatesLibrary::InternalOnViewportResized(FViewport* Viewport, uint32 Param)
{
    if (GEngine->GameViewport)
    {
        FVector2D ViewportSize;
        GEngine->GameViewport->GetViewportSize(ViewportSize);
        OnViewportResized.Broadcast(ViewportSize.X, ViewportSize.Y);
    }
}
