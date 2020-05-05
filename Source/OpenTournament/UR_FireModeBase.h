// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UR_FireModeBase.generated.h"

class IUR_FireModeBaseInterface;

/**
 *
 */
UCLASS(Abstract, Blueprintable, HideCategories = (Sockets, Tags, Cooking, AssetUserData, Collision))
class OPENTOURNAMENT_API UUR_FireModeBase : public UActorComponent
{
    GENERATED_BODY()

public:
    UUR_FireModeBase();

    /**
    * The blueprint components tree does not support re-ordering of components.
    * This can be used to order firemodes in actors implementing multiples (eg. weapon).
    */
    UPROPERTY(EditAnywhere, Category = "FireMode", Meta = (DisplayPriority = "1"))
    uint8 Index;

public:

    UPROPERTY(BlueprintReadOnly)
    TScriptInterface<IUR_FireModeBaseInterface> BaseInterface;

    UFUNCTION(BlueprintCallable)
    virtual void SetCallbackInterface(const TScriptInterface<IUR_FireModeBaseInterface>& CallbackInterface)
    {
        BaseInterface = CallbackInterface;
    }

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void StartFire();
    virtual void StartFire_Implementation() {}

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void StopFire();
    virtual void StopFire_Implementation() {}

    UFUNCTION(BlueprintCallable, BlueprintPure)
    virtual bool IsBusy()
    {
        return bIsBusy;
    }

    /**
    * Calculate (best effort) the time until this firemode becomes idle.
    * If mode is currently firing, calculate as if StopFire is being called.
    * If mode is on cooldown, return the remaining cooldown time.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure)
    float GetTimeUntilIdle();
    virtual float GetTimeUntilIdle_Implementation()
    {
        return 0.f;
    }

    /**
    * If firemode is on cooldown, return cooldown start time.
    * This may be used to calculate various things in conjunction with GetTimeUntilIdle(),
    * such as allowing weaponswitch at x% of the cooldown rather than waiting on the full cooldown.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure)
    float GetCooldownStartTime();
    virtual float GetCooldownStartTime_Implementation()
    {
        return 0.f;
    }

protected:

    UPROPERTY()
    bool bIsBusy;

    UFUNCTION(BlueprintCallable)
    virtual void SetBusy(bool bNewBusy);

};


/**
* NOTE:
*
* Blueprint does not support single-cast delegates or even multi-cast with by-ref parameters.
* That means it is straight up impossible to bind delegates/events with return values in BP.
*
* Some of our FireModes will need some data back from the controlling object,
* therefore our only option is to rely on interfaces.
*
* Each type of firemode may come with its own extended interface to communicate with the controlling object.
*/


/**
*
*/
UINTERFACE(Blueprintable)
class UUR_FireModeBaseInterface : public UInterface
{
    GENERATED_BODY()
};

class OPENTOURNAMENT_API IUR_FireModeBaseInterface
{
    GENERATED_BODY()

public:

    /**
    * Called when the firemode has changed its busy status.
    * Firemode becomes busy when it starts firing.
    * Firemode becomes idle when it is ready to refire (cooldown over).
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void FireModeChangedStatus(UUR_FireModeBase* FireMode);

    /**
    * Called when the firemode needs to know if it is allowed to fire, or how long it has to wait.
    *
    * Used during network synchronisation to ensure game integrity,
    * eg. to prevent server firing during weapon BringUp, or faster than fire interval across multiple firemodes.
    * Depending on value returned, server might delay the shot a bit, or discard the shot.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    float TimeUntilReadyToFire(UUR_FireModeBase* FireMode);
    virtual float TimeUntilReadyToFire_Implementation(UUR_FireModeBase* FireMode)
    {
        return FireMode->GetTimeUntilIdle();
    }

};
