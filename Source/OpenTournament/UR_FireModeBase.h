// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <CoreMinimal.h>
#include <Components/ActorComponent.h>

#include "UR_FireModeBase.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UFXSystemAsset;
class IUR_FireModeBaseInterface;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
UCLASS(Abstract, Blueprintable, HideCategories = (Sockets, Tags, Cooking, AssetUserData, Collision))
class OPENTOURNAMENT_API UUR_FireModeBase : public UActorComponent
{
    GENERATED_BODY()

public:
    UUR_FireModeBase();

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * The blueprint components tree does not support re-ordering of components.
    * This can be used to order firemodes in actors implementing multiples (eg. weapon).
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FireMode", Meta = (DisplayPriority = "1"))
    uint8 Index;

    /**
    * NOTE: I am currently pondering if it is actually worth it to support spinup in FireModeBasic.
    * It seems to me that any spinup firemode would be a very fast-firing firemode.
    * In that case, the firemode can be implemented as Continuous, even if it's projectiles.
    * The whole code would be a lot less complex if spinup was only in Continuous.
    * Investigate.
    */

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FireMode|SpinUp")
    float SpinUpTime;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FireMode|SpinUp")
    float SpinDownTime;

    /**
    * Configure to let the firemode enter idle state at a specific point during spindown.
    * At 0.0, the firemode will go idle after it has completely spun down.
    * At 1.0, the firemode can go idle as soon as StopFire is called, when spindown starts.
    * This impacts swapping firemodes, and weaponswap if CooldownDelaysPutdown > 0%.
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FireMode|SpinUp")
    float IdleAtSpinPercent;

public:
    /**
    * NOTE: The category "FireMode" is intended for configuring how the fire mode behaves.
    *
    * The category "Content" on the other hand is only here to provide some storage of the
    * most common properties, for the actors using firemodes. It's up to them to implement.
    * The idea was to avoid weapons having to declare additional arrays of data to go along with the firemodes.
    *
    * NOTE: This may change in the future, as I could see the interfaces providing actual
    * default implementations, by moving chunks of code from Weapon to them.
    * In that case, those default implementations would actually make use of the "Content" properties.
    *
    * That wouldn't technically hurt the modularity of the system, since each interface
    * method can still be overriden by the implementer.
    * If methods are overriden, then it's up to the implementer to use, or not,
    * these "Content" properties in whatever way they want.
    */

    /**
    * Minimum necessary ammo for weapon to allow firing this firemode at all.
    * If weapon ammo is below that value, it should click as out-of-ammo.
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Content")
    int32 InitialAmmoCost;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Content")
    float Spread;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Content")
    FName MuzzleSocketName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Content")
    UFXSystemAsset* MuzzleFlashTemplate;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Content")
    float MuzzleFlashScale;

public:
    UPROPERTY(BlueprintReadOnly)
    TScriptInterface<IUR_FireModeBaseInterface> BaseInterface;

    UFUNCTION(BlueprintCallable)
    virtual void SetCallbackInterface(const TScriptInterface<IUR_FireModeBaseInterface>& CallbackInterface)
    {
        BaseInterface = CallbackInterface;
    }

    /**
    * Wrapper for the spinup implementation.
    * Actual firing happens in StartFire.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void RequestStartFire();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void StartFire();

    virtual void StartFire_Implementation()
    {
    }

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void StopFire();

    UFUNCTION(BlueprintPure)
    virtual bool IsBusy()
    {
        return bIsBusy;
    }

    /**
    * Return true if this mode should be activatable independently of other fire modes.
    * Used for Zoom.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintPure)
    bool IsIndependentFireMode();

    virtual bool IsIndependentFireMode_Implementation()
    {
        return false;
    }


    /**
    * Calculate (best effort) the time until this firemode becomes idle.
    * If mode is currently firing, calculate as if StopFire is being called.
    * If mode is on cooldown, return the remaining cooldown time.
    * Takes into account spinup / IdleAtSpinPercent.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintPure)
    float GetTimeUntilIdle();

    /**
    * If firemode is on cooldown, return cooldown start time.
    * This may be used to calculate various things in conjunction with GetTimeUntilIdle(),
    * such as allowing weaponswitch at x% of the cooldown rather than waiting on the full cooldown.
    * Takes into account spinup / IdleAtSpinPercent.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintPure)
    float GetCooldownStartTime();

    /**
    * Request firemode to go to idle state whenever it sees opportunity.
    * This is used by weapon swap code to properly handle the following cases :
    * 1. Race conditions if cooldown delays putdown by 100%, firemode could loop before weapon could putdown.
    * 2. Requesting a swap while charging a shot, should not release the shot (which calling StopFire would do).
    * 3. Requesting a swap requiring weapon to spindown, and user tries to click again to spinup again.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void SetRequestIdle(bool bNewRequestIdle);

    virtual void SetRequestIdle_Implementation(bool bNewRequestIdle)
    {
        bRequestedIdle = bNewRequestIdle;   //prevent spinup
        StopFire(); //spindown
    }

    /**
    * Current spin percent (0.0 -> 1.0).
    */
    UFUNCTION(BlueprintPure)
    virtual float GetCurrentSpinUpValue();

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /**
    * True only on the machine that requested fire (RequestStartFire).
    * Not replicated.
    */
    UPROPERTY(BlueprintReadOnly)
    bool bRequestedFire;

    UPROPERTY()
    bool bIsBusy;

    UFUNCTION(BlueprintCallable)
    virtual void SetBusy(bool bNewBusy);

    UPROPERTY(BlueprintReadOnly)
    bool bRequestedIdle;

    //============================================================
    // SpinUp
    //============================================================

    UPROPERTY(BlueprintReadOnly)
    bool bFullySpinnedUp;

    UFUNCTION()
    virtual void SpinUp();

    FTimerHandle SpinUpTimerHandle;

    UFUNCTION()
    virtual void SpinUpCallback();

    UFUNCTION()
    virtual void SpinDown();

    FTimerHandle SpinDownIdleTimerHandle;
    FTimerHandle SpinDownTimerHandle;

    UFUNCTION()
    virtual void SpinDownIdleCallback();

    UFUNCTION()
    virtual void SpinDownCallback();

    // Replication

    UFUNCTION(Server, Reliable)
    void ServerSpinUp();

    FTimerHandle DelayedSpinUpTimerHandle;

    UFUNCTION(Server, Reliable)
    void ServerSpinDown();

    UPROPERTY(ReplicatedUsing = OnRep_IsSpinningUp, BlueprintReadOnly)
    bool bIsSpinningUpRep;

    UFUNCTION()
    virtual void OnRep_IsSpinningUp();

public:
    // UActorComponent tweaks

    /**
    * Please avoid using StartWithTickEnabled, which enables tick even when the component starts deactivated.
    * Use this to automatically enable tick only when component is activated.
    */
    UPROPERTY(EditAnywhere, Category = "ComponentTick")
    bool bAutoActivateTick;

    UPROPERTY()
    bool bWasActive;

    virtual void Activate(bool bReset = false) override;

    virtual void Deactivate() override;

    virtual void OnRep_IsActive() override;
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
    * Depending on the value returned, server might delay the shot a bit, or discard the shot.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    float TimeUntilReadyToFire(UUR_FireModeBase* FireMode);

    virtual float TimeUntilReadyToFire_Implementation(UUR_FireModeBase* FireMode)
    {
        return FireMode->GetTimeUntilIdle();
    }

    // Weapon returns this to indicate unacceptable state, and firing should be discarded.
#define TIMEUNTILFIRE_NEVER 10.f

    /**
    * When SpinUpTime > 0, called as the firemode starts spinning up.
    * Spinup can restart from the middle of a spindown.
    * The current spinup percentage (0.0 -> 1.0) is passed as parameter.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, BlueprintCallable)
    void BeginSpinUp(UUR_FireModeBase* FireMode, float CurrentSpinValue);

    /**
    * When SpinDownTime OR SpinUpTime > 0, called as the firemode starts spinning down.
    * Spindown can begin from the middle of a spinup.
    * The current spinup percentage (0.0 -> 1.0) is passed as parameter.
    * If SpinDownTime = 0, SpinDone will be called immediately after.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, BlueprintCallable)
    void BeginSpinDown(UUR_FireModeBase* FireMode, float CurrentSpinValue);

    /**
    * Called when either fully spinned up or fully spinned down.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, BlueprintCallable)
    void SpinDone(UUR_FireModeBase* FireMode, bool bFullySpinnedUp);
};
