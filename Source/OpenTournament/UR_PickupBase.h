// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "UR_PickupBase.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AActor;
class UPrimitiveComponent;
class ULocalMessage;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Pickup Base Actor
 */
UCLASS()
class OPENTOURNAMENT_API AUR_PickupBase : public AActor
{
    GENERATED_BODY()

    /////////////////////////////////////////////////////////////////////////////////////////////////
public:	
    AUR_PickupBase();

    /////////////////////////////////////////////////////////////////////////////////////////////////

    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
    class UCapsuleComponent* CapsuleComponent;

    /**
    * Assign to a component in the BP construction script.
    */
    UPROPERTY(BlueprintReadWrite)
    class USceneComponent* RotatingComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RotationRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BobbingHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BobbingSpeed;

    UPROPERTY(BlueprintReadWrite)
    FVector InitialRelativeLocation;

    /**
    * Pickup availability according to authority.
    * Updated via events, not actually replicated.
    */
    UPROPERTY(BlueprintReadWrite)
    bool bPickupAvailable;

    /**
    * Replicates the state of the pickup to late-joining players.
    * Replication is initial-only.
    * This slightly differs from bPickupAvailable as it has to handle player joining right inbetween MulticastWillRespawn and actual respawn.
    */
    UPROPERTY(Replicated, ReplicatedUsing = OnRep_bRepInitialPickupAvailable, BlueprintReadWrite)
    bool bRepInitialPickupAvailable;

    /**
    * Total respawn time from pickup to next pickup.
    * Should be greater or equal than PreRespawnEffectDuration.
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float RespawnTime;

    /**
    * Use this to start the respawn effect slightly before actual pickup respawn.
    * Useful if respawning has some sort of build-up effect.
    * Should be less or equal than RespawnTime.
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float PreRespawnEffectDuration;

    FTimerHandle RespawnTimerHandle;

    /**
    * Pickup availability according to client pickup predictions.
    * Used to check for prediction errors, and whether to play pick up effects or not.
    */
    UPROPERTY(BlueprintReadOnly)
    bool bPickupAvailableLocally;

    FTimerHandle PredictionErrorTimerHandle;

    /**
    * Initial spawn delay. For powerups.
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float InitialSpawnDelay;

    /**
    * Pickup message class.
    */
    UPROPERTY(EditAnywhere)
    TSubclassOf<ULocalMessage> PickupMessage;

protected:
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;
    UFUNCTION()
    virtual void OnRep_bRepInitialPickupAvailable();
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldSkipTick();

public:

    /**
    * Server / client.
    * Overlap callback.
    */
    UFUNCTION(BlueprintNativeEvent)
    void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    /**
    * Server / client.
    * Validate whether an overlapping actor can pickup this item.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure)
    bool AllowPickupBy(AActor* Other);

    /**
    * Authority only.
    * Give this item to the actor (already validated).
    */
    UFUNCTION(BlueprintNativeEvent)
    void GiveTo(AActor* Other);

    /**
    * Remote only.
    * Simulate picking up item on owning client.
    * This assumes AllowPickupBy() can reliably predict result on client side.
    * If that is not the case, return false in non-authority cases to avoid this.
    */
    UFUNCTION(BlueprintNativeEvent)
    void SimulateGiveTo(AActor* LocalClientActor);

    UFUNCTION()
    virtual void CheckClientPredictionError();

    /**
    * Broadcasted when pickup is being given away.
    */
    UFUNCTION(NetMulticast, Reliable)
    void MulticastPickedUp(AActor* Picker);

    /**
    * Client only.
    * Play pickup effects/sounds.
    */
    UFUNCTION(BlueprintCosmetic, BlueprintImplementableEvent)
    void PlayPickupEffects();

    /**
    * Client only.
    * Show or hide pickup availability.
    * Do not play effects or sounds here, this is used for init/late joins and error correction.
    */
    UFUNCTION(BlueprintCosmetic, BlueprintNativeEvent)
    void ShowPickupAvailable(bool bAvailable);

    /**
    * Authority only.
    * Only when RespawnTime - PreRespawnEffectDuration > 0.
    * Broadcasts MulticastWillRespawn.
    */
    UFUNCTION()
    virtual void PreRespawnTimer();

    /**
    * Broadcasted when pickup is about to respawn (RespawnTime minus PreRespawnEffectDuration).
    */
    UFUNCTION(NetMulticast, Reliable)
    void MulticastWillRespawn();

    /**
    * Client only.
    * Play respawn effects/sounds.
    */
    UFUNCTION(BlueprintCosmetic, BlueprintImplementableEvent)
    void PlayRespawnEffects();

    /**
    * Server / client.
    * Make pickup available again.
    */
    UFUNCTION()
    virtual void RespawnTimer();

    /**
    * Client only.
    * Return localized pickup text message.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, BlueprintCallable, BlueprintPure)
    FText GetPickupText(AActor* Picker);

    /**
    * Client only.
    * Return localized pickup name.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, BlueprintCallable, BlueprintPure)
    FText GetItemName();
};
