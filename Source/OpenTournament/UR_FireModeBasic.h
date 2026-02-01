// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "UR_FireModeBase.h"

#include "GameFramework/DamageType.h"

#include "UR_FunctionLibrary.h"
#include "UR_FireModeBasic.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AUR_Projectile;
class IUR_FireModeBasicInterface;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
* Stores information about a simulated shot,
* which will be passed to server for processing.
*
* This should be used at the very least to pass client exact fire location & rotation to server.
* Can also be used to implement clientside hitscan registration.
*
* It is intentionally very generic to support many sorts of hitscan implementations.
* eg. piercing rail, bouncing beam, seeded shotgun
*/
USTRUCT(BlueprintType)
struct FSimulatedShotInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> Vectors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<AActor*> Actors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Seed;

    FSimulatedShotInfo()
        : Seed(0)
    {
    }
};

/**
* Stores information about a hitscan shot,
* which will be passed to clients for reproducing accurate visuals.
*
* Intentionally also generic to support many sorts of hitscan implementations.
* eg. bouncing beam, seeded shotgun
*/
USTRUCT(BlueprintType)
struct FHitscanVisualInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> Vectors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Seed;

    FHitscanVisualInfo()
        : Seed(0)
    {
    }
};


/**
 *
 */
UCLASS(ClassGroup = (FireMode), Meta = (BlueprintSpawnableComponent))
class OPENTOURNAMENT_API UUR_FireModeBasic : public UUR_FireModeBase
{
    GENERATED_BODY()

public:
    UUR_FireModeBasic()
        : bIsHitscan(false)
        , FireSound(nullptr)
        , HitscanDamage(0)
        , BeamTemplate(nullptr)
        , BeamImpactTemplate(nullptr)
        , BeamImpactSound(nullptr)
        , LocalFireTime(0)
    {
        FireInterval = 1.0f;
        HitscanTraceDistance = 10000;
        BeamVectorParamName = FName(TEXT("BeamVector"));
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
    virtual void CooldownTimer();

    FTimerHandle CooldownTimerHandle;

    UFUNCTION(Server, Reliable)
    void ServerFire(const FSimulatedShotInfo& SimulatedInfo);

    UFUNCTION(BlueprintAuthorityOnly)
    virtual void AuthorityShot(const FSimulatedShotInfo& SimulatedInfo);

    FTimerHandle DelayedFireTimerHandle;

    UFUNCTION(NetMulticast, Reliable)
    void MulticastFired();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastFiredHitscan(const FHitscanVisualInfo& HitscanInfo);

    UFUNCTION()
    void LocalConfirmFired();
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
    /**
    * Simulate a non-hitscan shot on local client.
    * Do not play fire effects here.
    * Return netcode/hitreg info in the FSimulatedShotInfo struct, to be passed to server.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void SimulateShot(UUR_FireModeBasic* FireMode, FSimulatedShotInfo& OutSimulatedInfo);

    /**
    * Simulate a hitscan shot on local client.
    * Do not play fire effects or hitscan effects here.
    * Return netcode/hitreg info in the FSimulatedShotInfo struct, to be passed to server.
    * Also return hitscan visual info, to be passed directly to PlayHitscanEffects.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void SimulateHitscanShot(UUR_FireModeBasic* FireMode, FSimulatedShotInfo& OutSimulatedInfo, FHitscanVisualInfo& OutHitscanInfo);

    /**
    * Perform non-hitscan shot on authority side, using simulated info passed from client.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly, BlueprintCallable)
    void AuthorityShot(UUR_FireModeBasic* FireMode, const FSimulatedShotInfo& SimulatedInfo);

    /**
    * Perform hitscan shot on authority side, using simulated info passed from client.
    * Return visual info in the HitscanVisualInfo struct, to replicate visuals on other clients.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly, BlueprintCallable)
    void AuthorityHitscanShot(UUR_FireModeBasic* FireMode, const FSimulatedShotInfo& SimulatedInfo, FHitscanVisualInfo& OutHitscanInfo);

    /**
    * Play client-side fire effects (muzzle flash, sound, animations).
    * Called on owning client before simulating any shot,
    * and on remote clients when any shot is multicasted.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, BlueprintCallable)
    void PlayFireEffects(UUR_FireModeBasic* FireMode);

    /**
    * Play client-side hitscan effects (beam, impact).
    * Called on owning client after simulating a hitscan shot,
    * and on remote clients when a hitscan shot is multicasted.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, BlueprintCallable)
    void PlayHitscanEffects(UUR_FireModeBasic* FireMode, const FHitscanVisualInfo& HitscanInfo);
};
