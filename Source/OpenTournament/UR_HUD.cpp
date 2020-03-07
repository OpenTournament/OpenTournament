// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_HUD.h"

//UMG
#include "SlateBasics.h"
#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/UMGStyle.h"
#include "Runtime/UMG/Public/Slate/SObjectWidget.h"
#include "Runtime/UMG/Public/IUMGModule.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_HUD::AUR_HUD()
    : CrosshairTex(nullptr)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_HUD::DrawHUD()
{
    Super::DrawHUD();

    // Draw very simple crosshair
    DrawCrosshair();
}

void AUR_HUD::DrawCrosshair()
{
    if (CrosshairTex == nullptr)
    {
        return;
    }

    // find center of the Canvas
    const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

    // offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
    const FVector2D CrosshairDrawPosition((Center.X), (Center.Y + 20.0f));

    // draw the crosshair
    FCanvasTileItem TileItem(CrosshairDrawPosition, CrosshairTex->Resource, FLinearColor::White);
    TileItem.BlendMode = SE_BLEND_Translucent;
    Canvas->DrawItem(TileItem);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_HUD::Client_RestartHUD_Implementation()
{
    OnHUDRestart();
}
