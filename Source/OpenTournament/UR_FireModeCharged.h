// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UR_FireModeBasic.h"
#include "UR_FireModeCharged.generated.h"


/**
* Event dispatcher.
* Called at charge start (0) and every new charge level.
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChargeLevelSignature, UUR_FireModeCharged*, FireMode);


class IUR_FireModeChargedInterface;


/**
 * 
 */
UCLASS(ClassGroup = (FireMode), Meta = (BlueprintSpawnableComponent))
class OPENTOURNAMENT_API UUR_FireModeCharged : public UUR_FireModeBasic
{
	GENERATED_BODY()

public:
    UUR_FireModeCharged()
    {
        ChargeInterval = 0.25f;
        MaxChargeLevel = 4;
        MaxChargeHoldTime = -1;
    }

    /**
    * Interval between each ChargeLevel increment.
    */
    UPROPERTY(EditAnywhere, Category = "FireMode")
    float ChargeInterval;

    /**
    * Maximum charge level.
    */
    UPROPERTY(EditAnywhere, Category = "FireMode")
    int32 MaxChargeLevel;

    /**
    * Maximum hold time AFTER reaching full charge.
    * Use -1 for infinite.
    */
    UPROPERTY(EditAnywhere, Category = "FireMode")
    float MaxChargeHoldTime;

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
    */
    UPROPERTY(ReplicatedUsing = OnRep_InitialChargeLevel)
    int32 ChargeLevel;

    /**
    * Get current charge as a precise float value (0.0 ==> 1.0),
    * taking into account the current partial charge.
    * Use this when implementing continuous visuals from a tick faster than ChargeInterval.
    */
    UFUNCTION(BlueprintCallable, BlueprintPure)
    virtual float GetContinuousCharge();

    /**
    * Block at the current charge level.
    * The passed in MaxHoldTime parameter overrides default MaxChargeHoldTime.
    *
    * If MaxHoldTime is infinite (-1), the firemode will continue trying to charge,
    * but call OnChargeLevel with same charge level. This can be used to pause charging and resume later.
    *
    * If MaxHoldTime is real however, charging stops and will autofire after the delay.
    */
    UFUNCTION(BlueprintCallable)
    virtual void BlockNextCharge(float MaxHoldTime);

protected:

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /**
    * ChargeLevel replicates with condition INITIAL_ONLY.
    * This should occur when a player late joins and somebody is already charging,
    * the multicast begin charge was not triggered, so we should catch up from current value now.
    * This is especially important for modes which can be held charged forever.
    */
    UFUNCTION()
    virtual void OnRep_InitialChargeLevel();

    /**
    * Keep track of blocked charge level (set by BlockNextCharge) and replicate.
    */
    UPROPERTY(Replicated)
    int32 ChargePausedAt;

public:

    UPROPERTY(BlueprintAssignable)
    FChargeLevelSignature OnChargeLevel;

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

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void ChargeLevel(UUR_FireModeCharged* FireMode);

};

