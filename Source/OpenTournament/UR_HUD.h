// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <GameFramework/HUD.h>

#include "UR_HUD.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations


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
    AUR_HUD(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    /////////////////////////////////////////////////////////////////////////////////////////////////

    //~UObject interface
    virtual void PreInitializeComponents() override;
    //~End of UObject interface

    //~AActor interface
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    //~End of AActor interface

    //~AHUD interface
    virtual void GetDebugActorList(TArray<AActor*>& InOutList) override;
    virtual void ShowHUD() override;
    //~End of AHUD interface

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
};
