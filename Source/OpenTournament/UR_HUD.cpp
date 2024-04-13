// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_HUD.h"

//UMG
#include <GameFramework/GameplayMessageSubsystem.h>

#include "SlateBasics.h"
#include "UR_GameplayTags.h"
#include "Messages/CrosshairVerbMessage.h"
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

void AUR_HUD::BeginPlay()
{
    Super::BeginPlay();

    UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
    if (!HasAnyFlags(RF_ClassDefaultObject)  && MessageSubsystem.IsValidLowLevel())
    {
        MessageSubsystem.RegisterListener(URGameplayTags::Crosshair_Enable, this, &ThisClass::OnMessage_CrosshairEnable);
        MessageSubsystem.RegisterListener(URGameplayTags::Crosshair_Disable, this, &ThisClass::OnMessage_CrosshairDisable);
        MessageSubsystem.RegisterListener(URGameplayTags::Crosshair_HitRegister, this, &ThisClass::OnMessage_CrosshairHitRegister);
    }
}

void AUR_HUD::BeginDestroy()
{
    // UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
    // if (!HasAnyFlags(RF_ClassDefaultObject) && MessageSubsystem.IsValidLowLevel())
    // {
    //     MessageSubsystem.UnregisterListener(Handle_CrosshairEnable);
    //     MessageSubsystem.UnregisterListener(Handle_CrosshairDisable);
    //     MessageSubsystem.UnregisterListener(Handle_CrosshairHitRegister);
    // }

    Super::BeginDestroy();
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
    FCanvasTileItem TileItem(CrosshairDrawPosition, CrosshairTex->GetResource(), FLinearColor::White);
    TileItem.BlendMode = SE_BLEND_Translucent;
    Canvas->DrawItem(TileItem);
}

void AUR_HUD::OnMessage_CrosshairEnable(FGameplayTag InTag, const FCrosshairVerbMessage& InMessage)
{
    const FGameplayTag& RelevantTag = URGameplayTags::Crosshair_Enable;

    if (InTag.MatchesTag(RelevantTag)
        && InMessage.Verb.MatchesTag(RelevantTag)
        && IsValid(PlayerOwner))
    {
        if (auto Character = PlayerOwner->GetCharacter())
        {
            if (IsValid(InMessage.CrosshairData))
            {
                //CrosshairTex = InMessage.CrosshairData.Get()->Texture;
            }
        }
    }
}

void AUR_HUD::OnMessage_CrosshairDisable(FGameplayTag InTag, const FCrosshairVerbMessage& InMessage)
{
    const FGameplayTag& RelevantTag = URGameplayTags::Crosshair_Disable;

    if (InTag.MatchesTag(RelevantTag)
    && InMessage.Verb.MatchesTag(RelevantTag)
    && InMessage.Instigator == GetOwner())
    {
        //CrosshairTex = nullptr;
    }
}

void AUR_HUD::OnMessage_CrosshairHitRegister(FGameplayTag InTag, const FCrosshairVerbMessage& InMessage)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_HUD::Client_RestartHUD_Implementation()
{
    OnHUDRestart();
}
