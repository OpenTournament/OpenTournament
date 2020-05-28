// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

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
        if (bReset || !AIF_bActive)
        {
            Execute_AIF_InternalActivate(Cast<UObject>(this));
            AIF_bActive = true;
        }
    }

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void AIF_Deactivate(bool bReset = false);
    virtual void AIF_Deactivate_Implementation(bool bReset)
    {
        if (bReset || AIF_bActive)
        {
            Execute_AIF_InternalDeactivate(Cast<UObject>(this));
            AIF_bActive = false;
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
        return AIF_bActive;
    }

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    bool AIF_Toggle();
    virtual bool AIF_Toggle_Implementation()
    {
        Execute_AIF_SetActive(Cast<UObject>(this), !AIF_bActive, false);
        return AIF_bActive;
    }

protected:

    bool AIF_bActive;

    UFUNCTION(BlueprintNativeEvent)
    void AIF_InternalActivate();

    UFUNCTION(BlueprintNativeEvent)
    void AIF_InternalDeactivate();

};
