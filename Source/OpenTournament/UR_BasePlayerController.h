// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <CommonPlayerController.h>

#include "UR_BasePlayerController.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UMaterialParameterCollection;

class UUR_PlayerInput;
class UUR_UserSettings;
class UUR_MPC_Global;

/////////////////////////////////////////////////////////////////////////////////////////////////

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FViewTargetChangedSignature, AUR_BasePlayerController*, PC, AActor*, NewVT, AActor*, OldVT);

/**
 * Base class for MenuPlayerController and URPlayerController.
 */
UCLASS()
class OPENTOURNAMENT_API AUR_BasePlayerController
    : public ACommonPlayerController
{
    GENERATED_BODY()

public:
    AUR_BasePlayerController(const FObjectInitializer& ObjectInitializer);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    virtual void PostInitializeComponents() override;

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

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Reference to the global-game MaterialParameterCollection.
    */
    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UMaterialParameterCollection> MPC_GlobalGame;

    UPROPERTY(VisibleDefaultsOnly)
    TObjectPtr<UMaterialParameterCollection> MPC_GlobalGamePtr = nullptr;

    /**
    * Need to store an instance of this because modifying CDO causes side effects.
    */
    UPROPERTY(Transient)
    TObjectPtr<UUR_MPC_Global> UR_MPC_Global_Ref;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    UPROPERTY(Transient)
    TObjectPtr<UUR_UserSettings> UserSettings;

    UFUNCTION(BlueprintCosmetic)
    virtual void InitUserSettings();

    UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "Player|Settings")
    FORCEINLINE UUR_UserSettings* GetUserSettings() const
    {
        return UserSettings;
    }

    UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Player|Settings")
    virtual void ApplyAllSettings();

    UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Player|Settings")
    virtual void ApplyCameraSettings();

    UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Player|Settings")
    virtual void ApplyTeamColorSettings();

    UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Player|Settings")
    virtual void ApplyWeaponGroupSettings();

    /////////////////////////////////////////////////////////////////////////////////////////////////

    UPROPERTY(BlueprintAssignable)
    FViewTargetChangedSignature OnViewTargetChanged;
};
