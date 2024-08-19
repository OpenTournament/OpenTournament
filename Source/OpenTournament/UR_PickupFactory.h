// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "UR_PickupFactory.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class USceneComponent;
class AUR_Pickup;
class UFXSystemAsset;
class USoundBase;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * PickupFactory base actor class.
 * Spawns and respawns actors of class UR_Pickup.
 */
UCLASS(Abstract)
class OPENTOURNAMENT_API AUR_PickupFactory : public AActor
{
    GENERATED_BODY()

    /////////////////////////////////////////////////////////////////////////////////////////////////
public:
    AUR_PickupFactory();

    /////////////////////////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
    virtual void CheckForErrors() override;
#endif

    /**
    * Spawned pickup will be attached to this component.
    * By default this points to the root component of the factory.
    * In BP subclasses, you can assign this to a different component during Construction Script.
    *
    * This is the component that will be updated if RotationRate or Bobbing is enabled.
    */
    UPROPERTY(BlueprintReadWrite)
    USceneComponent* AttachComponent;

#if WITH_EDITORONLY_DATA
    /**
    * EditorOnly - pickup preview static mesh component.
    * Default setup happens just after Construction script.
    * It can be extended/overriden in function SetupEditorPreview.
    */
    UPROPERTY(Transient, BlueprintReadOnly)
    UStaticMeshComponent* EditorPreview;
#endif

    /**
    * Rotates AttachComponent at a fixed rate.
    */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    float RotationRate;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    float BobbingHeight;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    float BobbingSpeed;

    /**
    * Saved initial relative location of AttachComponent.
    * Used to calculate bobbing.
    *
    * If your base uses bobbing and you change AttachComponent relative location, make sure to update this.
    */
    UPROPERTY(BlueprintReadWrite)
    FVector InitialRelativeLocation;

    /**
    * Total respawn time from pickup to next pickup.
    * Should be greater or equal than PreRespawnEffectDuration.
    */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float RespawnTime;

    /**
    * Use this to start the respawn effect slightly before actual pickup respawn.
    * Useful if respawning has some sort of build-up effect.
    * Should be less or equal than RespawnTime.
    */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float PreRespawnEffectDuration;

    FTimerHandle RespawnTimerHandle;

    /**
    * Initial spawn delay. For powerups.
    */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float InitialSpawnDelay;

    /**
    * Class of the pickup to spawn.
    * Logic can be extended/overriden in CreatePickup().
    *
    * Can be used on client for cosmetic purposes (holo...)
    * Replicated initial-only to let gamemode replace pickups on startup.
    */
    UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_PickupClass)
    TSubclassOf<AUR_Pickup> PickupClass_Internal;

    UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_PickupClass_Soft)
    TSoftClassPtr<AUR_Pickup> PickupClass_Soft;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    UFXSystemAsset* RespawnEffect;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    USoundBase* RespawnSound;

    /**
    * Spawned pickup instance.
    */
    UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing = OnRep_Pickup)
    AUR_Pickup* Pickup;

protected:
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;
    virtual void Reset() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldSkipTick();

    UFUNCTION()
    virtual void OnRep_PickupClass();
    UFUNCTION()
    virtual void OnRep_PickupClass_Soft();
    UFUNCTION()
    virtual void OnRep_Pickup();

public:

    /**
    * Override this to alter the EditorPreview static mesh component.
    * This is called in editor after every construction / property update.
    */
    UFUNCTION(BlueprintNativeEvent)
    void SetupEditorPreview(UStaticMeshComponent* PreviewComp);

    /**
    * Factor method for setting up respawn timer or initial spawn delay,
    * taking into account the pre-respawn effect duration.
    * Handles all zero cases.
    */
    UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
    virtual void BeginRespawnTimer(float InRespawnTime);

    /**
    * Create and setup pickup.
    */
    UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
    virtual void SpawnPickup();

    /**
    * Returns pickup class to spawn.
    * Can be overriden to decide of pickup class with dynamic logic.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintPure)
    TSubclassOf<AUR_Pickup> GetPickupClass();
    virtual TSubclassOf<AUR_Pickup> GetPickupClass_Implementation()
    {
        return PickupClass_Internal;
    }

    UFUNCTION(BlueprintCallable)
    virtual void SetPickupClass(TSubclassOf<AUR_Pickup> NewClass)
    {
        PickupClass_Internal = NewClass;
    }

    /**
    * Override this to set properties on new pickup before it is added to the scene.
    * WARNING: This is called during deferred spawn, so scene-related things will not work here.
    * This is necessary because if player is standing on spawn, pickup may be instantly deleted.
    */
    UFUNCTION(BlueprintNativeEvent)
    void PreInitializePickup(AUR_Pickup* UpcomingPickup);
    virtual void PreInitializePickup_Implementation(AUR_Pickup* UpcomingPickup) {}

    /**
    * Adjust pickup after is has been fully spawned.
    * Only called if pickup has not been destroyed (picked up) on spawn.
    */
    UFUNCTION(BlueprintNativeEvent)
    void PostInitializePickup(AUR_Pickup* NewPickup);
    virtual void PostInitializePickup_Implementation(AUR_Pickup* NewPickup) {}

    /**
    * Bound to pickup's OnPickedUp event dispatcher.
    * Used to kick off respawn timer.
    */
    UFUNCTION(BlueprintNativeEvent)
    void OnPickupPickedUp(AUR_Pickup* Other, APawn* Recipient);

    /**
    * Authority only.
    * Only when RespawnTime - PreRespawnEffectDuration > 0.
    * Broadcasts MulticastWillRespawn.
    */
    UFUNCTION(BlueprintAuthorityOnly)
    virtual void PreRespawnTimer();

    /**
    * Broadcast when pickup is about to respawn (RespawnTime minus PreRespawnEffectDuration).
    *
    * Unreliable as we don't want this to force a full actor replication when not relevant.
    * That might cause missing respawn effects in some cases. TBD.
    */
    UFUNCTION(NetMulticast, Unreliable)
    void MulticastWillRespawn();

    /**
    * Client only. Triggered at RespawnTime minus PreRespawnEffectDuration.
    * Play respawn effects/sounds.
    */
    UFUNCTION(BlueprintCosmetic, BlueprintNativeEvent)
    void PlayRespawnEffects();

    /**
    * Client only.
    * Show or hide pickup availability.
    * Do not play effects or sounds here, this is used for general state which can replicate anytime (late join, relevancy).
    */
    UFUNCTION(BlueprintCosmetic, BlueprintNativeEvent)
    void ShowPickupAvailable(bool bAvailable);

    /**
    * Authority only.
    * Make pickup available.
    */
    UFUNCTION(BlueprintAuthorityOnly)
    virtual void RespawnTimer();

    /**
    * Setter to ensure triggering OnRep & ShowPickupAvailable in standalone.
    */
    UFUNCTION()
    virtual void SetPickup(AUR_Pickup* NewPickup);

    UFUNCTION()
    virtual void OnPickupDestroyed(AActor* DestroyedActor);
};
