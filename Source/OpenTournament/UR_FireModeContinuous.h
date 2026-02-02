// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "UR_FireModeBase.h"
#include "UR_FunctionLibrary.h"
#include "UR_FireModeContinuous.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UAudioComponent;
class IUR_FireModeContinuousInterface;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
* NOTE:
*
* The firemode should provide boilerplate for implementing client side hit reg,
* without overloading the network with tick based RPCs.
*
* Thinking about following solution :
* - Server counts hits on its side
* - Client counts hits on its side
* - Every increment, client RPCs server with an unreliable call, with its current count
* - Server stores the last count received from client
* - When server receives a new count, compare to previous (new hits = count - last count)
* - Authorize all new hits up until client count > server count
*
* Problem: The final unreliable RPC(s) that didn't reach server, will be discarded.
* Problem: Alternating between two targets.
* Considering some packets will be lost, server essentially receives data in "chunks" (eg. new hits > 1).
* Possible solution: add additional reliable calls sent at a lower rate, with cumulated hits per player
*/

/*
* Second approach:
*
* Send more of a history with each RPC, but let server define checkpoints to reduce it.
* - Make a variable "ServerReceivedHitCount" replicated, so client knows what server received from him.
* - Every increment (or tick), client sends his entire history of new hits accumulated since ServerReceivedHitCount.
*
* ++ Should fix alternating between two targets.
* ++ If it continues sending repeatedly until server is up to date, should also fix the "last packet" problem.
* ++ If we add timestamps, server doesn't need to count, as it can rewind and check each hit.
*
* I like this approach but need to ensure it doesn't spam too much data.
* Timestamps might be a luxury we cannot afford, because it means sending each hit individually.
* Player with 100 ping hitting continuously will send a history of minimum 13 hits, without even packet loss.
* Without timestamps, we can simply send the hit count for each opponent. Much lighter.
* Investigate.
*/

USTRUCT(BlueprintType)
struct FStoredTargetHitCount
{
    GENERATED_BODY()

    UPROPERTY()
    AActor* Target;

    UPROPERTY()
    uint8 HitCount;

    FStoredTargetHitCount()
        : Target(nullptr)
        , HitCount(0)
    {
    }
};

/**
 *
 */
UCLASS(ClassGroup = (FireMode), Meta = (BlueprintSpawnableComponent))
class OPENTOURNAMENT_API UUR_FireModeContinuous : public UUR_FireModeBase
{
    GENERATED_BODY()

public:
    UUR_FireModeContinuous()
    {
        PrimaryComponentTick.bCanEverTick = true;
        //PrimaryComponentTick.TickInterval = 0.050f;
        PrimaryComponentTick.bStartWithTickEnabled = false;

        HitCheckInterval = 0.050f;

        TraceDistance = 1000;
        AmmoCostPerSecond = 1.f;
        BeamVectorParamName = FName(TEXT("BeamVector"));
        BeamImpactNormalParamName = FName(TEXT("ImpactNormal"));
    }

    /**
    * Behaves like TickInterval but controls the rate of the ContinuousHitCheck callbacks.
    * This should be set to a fixed value so DPS does not depend on framerate.
    *
    * The reason we do not use TickInterval is because you still need a "real" tick to update visuals,
    * which is what the PlayContinuousEffects callback is for.
    */
    UPROPERTY(EditAnywhere, Category = "FireMode")
    float HitCheckInterval;

public:
    UPROPERTY(EditAnywhere, Category = "Content")
    float TraceDistance;

    UPROPERTY(EditAnywhere, Category = "Content")
    float Damage;

    UPROPERTY(EditAnywhere, Category = "Content")
    float AmmoCostPerSecond;

    UPROPERTY(EditAnywhere, Category = "Content")
    TSubclassOf<UDamageType> DamageType;

    UPROPERTY(EditAnywhere, Category = "Content")
    UFXSystemAsset* BeamTemplate;

    UPROPERTY(EditAnywhere, Category = "Content")
    FName BeamVectorParamName;

    UPROPERTY(EditAnywhere, Category = "Content")
    FName BeamImpactNormalParamName;

    UPROPERTY(EditAnywhere, Category = "Content")
    USoundBase* FireLoopSound;

    UPROPERTY(EditAnywhere, Category = "Content")
    USoundBase* FireEndSound;

    UPROPERTY(EditAnywhere, Category = "Content")
    USoundBase* BeamImpactSound;

    UPROPERTY(BlueprintReadWrite, Category = "Content|Runtime")
    UFXSystemComponent* BeamComponent;

    UPROPERTY(BlueprintReadWrite, Category = "Content|Runtime")
    UAudioComponent* FireLoopAudioComponent;

    UPROPERTY(BlueprintReadWrite, Category = "Content|Runtime")
    UAudioComponent* ImpactLoopAudioComponent;

    UPROPERTY(BlueprintReadWrite, Category = "Content|Runtime")
    float AmmoCostAccumulator;

public:
    UPROPERTY(BlueprintReadOnly)
    TScriptInterface<IUR_FireModeContinuousInterface> ContinuousInterface;

    virtual void SetCallbackInterface(const TScriptInterface<IUR_FireModeBaseInterface>& CallbackInterface) override
    {
        Super::SetCallbackInterface(CallbackInterface);
        UUR_FunctionLibrary::CastScriptInterface<IUR_FireModeBaseInterface, IUR_FireModeContinuousInterface>(CallbackInterface, ContinuousInterface);
    }

    virtual void RequestStartFire_Implementation() override;

    virtual void StartFire_Implementation() override;

    virtual void StopFire_Implementation() override;

    virtual void SpinDown() override;

    virtual float GetTimeUntilIdle_Implementation() override;

protected:
    /*
    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        //bStartWithTickEnabled=false doesn't work...
        SetComponentTickEnabled(false);
    }
    */

    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY()
    float DeltaTimeAccumulator;
};

UINTERFACE(Blueprintable)
class UUR_FireModeContinuousInterface : public UUR_FireModeBaseInterface
{
    GENERATED_BODY()
};

class OPENTOURNAMENT_API IUR_FireModeContinuousInterface : public IUR_FireModeBaseInterface
{
    GENERATED_BODY()

public:
    /**
    * Called every HitCheck "tick" (according to HitCheckInterval) on owner client.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void SimulateContinuousHitCheck(UUR_FireModeContinuous* FireMode);

    /**
    * Called when firing starts on authority, before first HitCheck.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly, BlueprintCallable)
    void AuthorityStartContinuousFire(UUR_FireModeContinuous* FireMode);

    /**
    * Called every HitCheck "tick" (according to HitCheckInterval) on authority.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly, BlueprintCallable)
    void AuthorityContinuousHitCheck(UUR_FireModeContinuous* FireMode);

    /**
    * Called when firing stops on authority, after last HitCheck.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly, BlueprintCallable)
    void AuthorityStopContinuousFire(UUR_FireModeContinuous* FireMode);

    /**
    * Called when starting continuous firing, on all clients.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, BlueprintCallable)
    void StartContinuousEffects(UUR_FireModeContinuous* FireMode);

    /**
    * Called every tick during continuous firing on all clients.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, BlueprintCallable)
    void UpdateContinuousEffects(UUR_FireModeContinuous* FireMode, float DeltaTime);

    /**
    * Called when continuous firing stops, on all clients.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, BlueprintCallable)
    void StopContinuousEffects(UUR_FireModeContinuous* FireMode);
};
