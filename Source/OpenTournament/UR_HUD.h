// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"

#include "UR_HUD.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations

class UUserWidget;

/////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Core HUD Class
 */
UCLASS()
class OPENTOURNAMENT_API AUR_HUD : public AHUD
{
    GENERATED_BODY()

public:

    AUR_HUD();

    /**
    * Primary draw call for the HUD
    */
    virtual void DrawHUD() override;

    /**
    * Crosshair asset pointer
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HUD")
    class UTexture2D* CrosshairTex;

protected:
    void DrawCrosshair();
};
