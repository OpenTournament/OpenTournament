// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UR_FireModeBase.h"
#include "UR_FunctionLibrary.h"
#include "UR_FireModeBasic.generated.h"

class AUR_Projectile;
class UFXSystemAsset;

/**
* Single delegate.
* Simulate shot on local client.
* Do not play fire effects, the appropriate delegate will be called right after.
* Return netcode/hitreg info in the FSimulatedShotInfo struct, to be passed to server.
*/
DECLARE_DELEGATE_TwoParams(FSimulateShotDelegate, UUR_FireModeBasic*, FSimulatedShotInfo&);

/**
* Event dispatcher.
* Firemode just fired, and clients should play fire effects (muzzle flash, sound, animations).
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayFireEffectsSignature, UUR_FireModeBasic*, FireMode);

/**
* Single delegate.
* Perform shot on authority side, using the info passed from client.
*/
DECLARE_DELEGATE_TwoParams(FAuthorityShotDelegate, UUR_FireModeBasic*, const FSimulatedShotInfo&);

/**
* Single delegate.
* Perform hitscan shot on authority side, using the info passed from client.
* Return visual info in the HitscanVisualInfo struct, to replicate visuals on all clients.
*/
DECLARE_DELEGATE_ThreeParams(FAuthorityHitscanShotDelegate, UUR_FireModeBasic*, const FSimulatedShotInfo&, FHitscanVisualInfo&);

/**
* Event dispatcher.
* Hitscan just fired, and clients should play hitscan effects (beam, impact).
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPlayHitscanEffectsSignature, UUR_FireModeBasic*, FireMode, const FHitscanVisualInfo&, HitscanInfo);

class IUR_FireModeBasicInterface;

/**
 * 
 */
UCLASS(ClassGroup = (FireMode), Meta = (BlueprintSpawnableComponent))
class OPENTOURNAMENT_API UUR_FireModeBasic : public UUR_FireModeBase
{
	GENERATED_BODY()

public:
    UUR_FireModeBasic()
    {
        FireInterval = 1.0f;
    }

    UPROPERTY(EditAnywhere, Category = "FireMode")
    float FireInterval;

    /**
    * Dictates whether to trigger the Hitscan callbacks or the regular ones.
    * See delegates definitions.
    */
    UPROPERTY(EditAnywhere, Category = "FireMode")
    bool bIsHitscan;

public:

    UPROPERTY(EditAnywhere, Category = "Content")
    FName MuzzleSocketName;

    UPROPERTY(EditAnywhere, Category = "Content")
    UParticleSystem* MuzzleFlashFX;

    UPROPERTY(EditAnywhere, Category = "Content")
    USoundBase* FireSound;

    UPROPERTY(EditAnywhere, Category = "Content|Projectile")
    TSubclassOf<AUR_Projectile> ProjectileClass;

    UPROPERTY(EditAnywhere, Category = "Content|Hitscan")
    float HitscanTraceDistance;

    UPROPERTY(EditAnywhere, Category = "Content|Hitscan")
    float HitscanDamage;

    UPROPERTY(EditAnywhere, Category = "Content|Hitscan")
    TSubclassOf<UDamageType> HitscanDamageType;

    UPROPERTY(EditAnywhere, Category = "Content|Hitscan")
    UFXSystemAsset* BeamTemplate;

    UPROPERTY(EditAnywhere, Category = "Content|Hitscan")
    FName BeamVectorParamName;

    UPROPERTY(EditAnywhere, Category = "Content|Hitscan")
    UParticleSystem* BeamImpactTemplate;

    UPROPERTY(EditAnywhere, Category = "Content|Hitscan")
    USoundBase* BeamImpactSound;

public:

    UPROPERTY(BlueprintReadOnly)
    TScriptInterface<IUR_FireModeBasicInterface> BasicInterface;

    virtual void SetCallbackInterface(const TScriptInterface<IUR_FireModeBaseInterface>& CallbackInterface) override
    {
        Super::SetCallbackInterface(CallbackInterface);
        UUR_FunctionLibrary::CastScriptInterface<IUR_FireModeBaseInterface, IUR_FireModeBasicInterface>(CallbackInterface, BasicInterface);
    }

    virtual void StartFire_Implementation() override;
    virtual float GetTimeUntilIdle_Implementation() override;
    virtual float GetCooldownStartTime_Implementation() override;

protected:

    /**
    * Used to calculate server response time to our ServerFire() call, and adjust fire loop.
    * Owner client only.
    */
    UPROPERTY()
    float LocalFireTime;

    UFUNCTION()
    void CooldownTimer();

    FTimerHandle CooldownTimerHandle;

    UFUNCTION(Server, Reliable)
    void ServerFire(const FSimulatedShotInfo& SimulatedInfo);

    FTimerHandle DelayedFireTimerHandle;

    UFUNCTION(NetMulticast, Reliable)
    void MulticastFired();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastFiredHitscan(const FHitscanVisualInfo& HitscanInfo);

    UFUNCTION()
    void LocalConfirmFired();

public:

    FSimulateShotDelegate SimulateShotDelegate;
    FAuthorityShotDelegate AuthorityShotDelegate;
    FAuthorityHitscanShotDelegate AuthorityHitscanShotDelegate;

    UPROPERTY(BlueprintAssignable)
    FPlayFireEffectsSignature OnPlayFireEffects;

    UPROPERTY(BlueprintAssignable)
    FPlayHitscanEffectsSignature OnPlayHitscanEffects;

};


UINTERFACE(Blueprintable)
class UUR_FireModeBasicInterface : public UUR_FireModeBaseInterface
{
    GENERATED_BODY()
};

class OPENTOURNAMENT_API IUR_FireModeBasicInterface : public IUR_FireModeBaseInterface
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void SimulateShot(UUR_FireModeBasic* FireMode, FSimulatedShotInfo& OutSimulatedInfo);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void SimulateHitscanShot(UUR_FireModeBasic* FireMode, FSimulatedShotInfo& OutSimulatedInfo, FHitscanVisualInfo& OutHitscanInfo);

    UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly, BlueprintCallable)
    void AuthorityShot(UUR_FireModeBasic* FireMode, const FSimulatedShotInfo& SimulatedInfo);
   
    UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly, BlueprintCallable)
    void AuthorityHitscanShot(UUR_FireModeBasic* FireMode, const FSimulatedShotInfo& SimulatedInfo, FHitscanVisualInfo& OutHitscanInfo);

    UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, BlueprintCallable)
    void PlayFireEffects(UUR_FireModeBasic* FireMode);

    UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, BlueprintCallable)
    void PlayHitscanEffects(UUR_FireModeBasic* FireMode, const FHitscanVisualInfo& HitscanInfo);

};
