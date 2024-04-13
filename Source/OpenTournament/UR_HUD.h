// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"

#include "UR_HUD.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations

struct FCrosshairVerbMessage;
struct FGameplayTag;
class UUserWidget;
class UTexture2D;

/////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Core HUD Class
 */
UCLASS()
class OPENTOURNAMENT_API AUR_HUD : public AHUD
{
    GENERATED_BODY()

    /////////////////////////////////////////////////////////////////////////////////////////////////

public:
    AUR_HUD();

    /////////////////////////////////////////////////////////////////////////////////////////////////

    virtual void BeginPlay() override;
    virtual void BeginDestroy() override;

    /**
    * Primary draw call for the HUD
    */
    virtual void DrawHUD() override;

    /**
    * Crosshair asset pointer
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HUD")
    UTexture2D* CrosshairTex;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * [Client] Call to restart the HUD
    */
    UFUNCTION(Client, Reliable, BlueprintCallable, Category = "HUD")
    void Client_RestartHUD();

    /**
    * BP-Implementable Event for Restarting HUD
    */
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "HUD")
    void OnHUDRestart();

protected:
    void DrawCrosshair();


#pragma region MessageSubsystem

    void OnMessage_CrosshairEnable(FGameplayTag InTag, const FCrosshairVerbMessage& InMessage);
    void OnMessage_CrosshairDisable(FGameplayTag InTag, const FCrosshairVerbMessage& InMessage);
    void OnMessage_CrosshairHitRegister(FGameplayTag InTag, const FCrosshairVerbMessage& InMessage);

#pragma endregion MessageSubsystem

    UPROPERTY()
    TArray<UUserWidget*> HUDWidgets;
};
