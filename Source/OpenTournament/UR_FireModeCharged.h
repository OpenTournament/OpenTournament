// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UR_FireModeBasic.h"
#include "UR_FireModeCharged.generated.h"

class IUR_FireModeChargedInterface;

/**
 * 
 */
UCLASS(ClassGroup = (FireMode), Meta = (BlueprintSpawnableComponent), HideCategories = ("FireMode|SpinUp"))
class OPENTOURNAMENT_API UUR_FireModeCharged : public UUR_FireModeBasic
{
	GENERATED_BODY()

public:
    UUR_FireModeCharged()
    {
        MaxChargeLevel = 5;
        ChargeInterval = 0.25f;
        MaxChargeHoldTime = -1;
    }

    /**
    * Maximum charge level. Charging starts at 1.
    */
    UPROPERTY(EditAnywhere, Category = "FireMode")
    int32 MaxChargeLevel;

    /**
    * Interval between each ChargeLevel increment.
    * First charge (1) is immediate.
    */
    UPROPERTY(EditAnywhere, Category = "FireMode")
    float ChargeInterval;

    /**
    * Maximum hold time AFTER reaching full charge.
    * Use -1 for infinite.
    */
    UPROPERTY(EditAnywhere, Category = "FireMode")
    float MaxChargeHoldTime;

public:

    /**
    * Hitscan damage of an uncharged shot.
    */
    UPROPERTY(EditAnywhere, Category = "Content|Hitscan")
    float HitscanDamageMin;

    /**
    * Hitscan damage of a fully charged shot.
    */
    UPROPERTY(EditAnywhere, Category = "Content|Hitscan")
    float HitscanDamageMax;

public:

    UPROPERTY(BlueprintReadOnly)
    TScriptInterface<IUR_FireModeChargedInterface> ChargedInterface;

    virtual void SetCallbackInterface(const TScriptInterface<IUR_FireModeBaseInterface>& CallbackInterface) override
    {
        Super::SetCallbackInterface(CallbackInterface);
        UUR_FunctionLibrary::CastScriptInterface<IUR_FireModeBaseInterface, IUR_FireModeChargedInterface>(CallbackInterface, ChargedInterface);
    }

    virtual void StartFire_Implementation() override;
    virtual void StopFire_Implementation() override;
    virtual float GetTimeUntilIdle_Implementation() override;
    virtual float GetCooldownStartTime_Implementation() override;

public:

    /**
    * Current charge level.
    * Starts at 1, ends at MaxChargeLevel.
    */
    UPROPERTY(ReplicatedUsing = OnRep_InitialChargeLevel, BlueprintReadOnly)
    int32 ChargeLevel;

    /**
    * Get current charge as a precise float value (0.0 ==> 1.0).
    * Can take into account the current partial charge.
    * Use this when implementing continuous visuals from a tick faster than ChargeInterval.
    */
    UFUNCTION(BlueprintCallable, BlueprintPure)
    virtual float GetTotalChargePercent(bool bIncludePartial = true);

    /**
    * Block at the current charge level.
    * The passed in MaxHoldTime parameter overrides default MaxChargeHoldTime.
    *
    * If MaxHoldTime is infinite (-1), the firemode will continue trying to charge, but it will
    * call OnChargeLevel with same charge level. This can be used to pause charging and resume later.
    *
    * If MaxHoldTime is real however, charging stops and will autofire after the delay.
    */
    UFUNCTION(BlueprintCallable)
    virtual void BlockNextCharge(float MaxHoldTime);

    // Override to avoid calling StopFire() which would release the charge.
    virtual void SetRequestIdle_Implementation(bool bNewRequestIdle)
    {
        bRequestedIdle = bNewRequestIdle;
    }

protected:

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY()
    float LocalStartChargeTime;

    /**
    * Keep track of blocked charge level (set by BlockNextCharge) and replicate.
    */
    UPROPERTY(Replicated)
    int32 ChargePausedAt;

    UFUNCTION()
    virtual void StartCharge();

    UFUNCTION()
    virtual void NextChargeLevel();

    FTimerHandle ChargeTimerHandle;

    UFUNCTION()
    virtual void SetHoldTimeout(float MaxHoldTime);

    UFUNCTION()
    virtual void HoldTimeout();

    UFUNCTION()
    virtual void ReleaseCharge(bool bOvercharged = false);

    virtual void CooldownTimer() override;

    // Replication

    UFUNCTION(Server, Reliable)
    void ServerStartCharge();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastStartCharge();

    /**
    * ChargeLevel replicates with condition INITIAL_ONLY.
    * This should occur when a player late joins and somebody is already charging,
    * the multicast begin charge was not triggered, so we should catch up from current value now.
    * This is especially important for modes which can be held charged forever.
    */
    UFUNCTION()
    virtual void OnRep_InitialChargeLevel();

    virtual void ServerFire_Implementation(const FSimulatedShotInfo& SimulatedInfo) override;
    virtual void MulticastFired_Implementation() override;
    virtual void MulticastFiredHitscan_Implementation(const FHitscanVisualInfo& HitscanInfo) override;

};


UINTERFACE(Blueprintable)
class UUR_FireModeChargedInterface : public UUR_FireModeBasicInterface
{
    GENERATED_BODY()
};

class OPENTOURNAMENT_API IUR_FireModeChargedInterface : public IUR_FireModeBasicInterface
{
    GENERATED_BODY()

public:

    /**
    * Called at charge start (1) and every new charge level.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void ChargeLevel(UUR_FireModeCharged* FireMode, int32 ChargeLevel, bool bWasPaused);

    /**
    * Called when a shot is about to autofire due to overcharge.
    * SimulateShot will be called right after in the same frame.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, BlueprintCallable)
    void Overcharged(UUR_FireModeCharged* FireMode);

};

