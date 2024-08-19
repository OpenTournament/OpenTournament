// Copyright (c) Open Tournament Project, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UR_ActivatableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UUR_ActivatableInterface : public UInterface
{
	GENERATED_BODY()
};

// @! TODO Need to evaluate this design as this interface has state on it, not sure we want this
/**
 * Represents an object that can be activated, deactivated, toggled, and tracks its status.
 *
 * The prefix AIF_ is used to avoid clashing with other existing methods due to common names.
 * For example, Components already have Activate/Deactivate/IsActive methods.
 *
 * This might sound weird but I already have an use case for it.
 * The zoom firemode can be activated/deactivated as a firemode component,
 * but can also activate/deactivate the zoom scope, which are two different things.
 */
class OPENTOURNAMENT_API IUR_ActivatableInterface
{
	GENERATED_BODY()

public:

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void AIF_Activate(bool bReset = false);
    virtual void AIF_Activate_Implementation(bool bReset)
    {
        if (bReset || !bAIF_BActive)
        {
            Execute_AIF_InternalActivate(Cast<UObject>(this));
            bAIF_BActive = true;
        }
    }

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void AIF_Deactivate(bool bReset = false);
    virtual void AIF_Deactivate_Implementation(bool bReset)
    {
        if (bReset || bAIF_BActive)
        {
            Execute_AIF_InternalDeactivate(Cast<UObject>(this));
            bAIF_BActive = false;
        }
    }

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void AIF_SetActive(bool bNewActive, bool bReset = false);
    virtual void AIF_SetActive_Implementation(bool bNewActive, bool bReset)
    {
        bNewActive ? Execute_AIF_Activate(Cast<UObject>(this), bReset) : Execute_AIF_Deactivate(Cast<UObject>(this), bReset);
    }

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    bool AIF_IsActive();
    virtual bool AIF_IsActive_Implementation()
    {
        return bAIF_BActive;
    }

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    bool AIF_Toggle();
    virtual bool AIF_Toggle_Implementation()
    {
        Execute_AIF_SetActive(Cast<UObject>(this), !bAIF_BActive, false);
        return bAIF_BActive;
    }

protected:

    bool bAIF_BActive = false;

    UFUNCTION(BlueprintNativeEvent)
    void AIF_InternalActivate();

    UFUNCTION(BlueprintNativeEvent)
    void AIF_InternalDeactivate();

};
