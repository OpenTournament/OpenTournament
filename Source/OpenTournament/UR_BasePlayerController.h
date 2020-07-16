// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UR_BasePlayerController.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UUR_PlayerInput;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Base class for MenuPlayerController and URPlayerController.
 */
UCLASS()
class OPENTOURNAMENT_API AUR_BasePlayerController : public APlayerController
{
    GENERATED_BODY()

public:

    AUR_BasePlayerController(const FObjectInitializer& ObjectInitializer);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    virtual void InitInputSystem() override;

    virtual void SpawnPlayerCameraManager() override;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    UFUNCTION(BlueprintCallable, Category = "PlayerController|Input")
    UUR_PlayerInput* GetPlayerInput() const;

    /**
    * Return to main menu.
    */
    UFUNCTION(Exec, BlueprintCallable, BlueprintCosmetic)
    void ReturnToMainMenu();

    /**
    * User configured FOV.
    */
    UPROPERTY(Config, BlueprintReadOnly)
    int32 ConfiguredFOV;

    UFUNCTION(Exec, BlueprintCallable, BlueprintCosmetic)
    void SetConfiguredFOV(int32 NewFOV);

    UFUNCTION()
    void ClampConfiguredFOV();

};
