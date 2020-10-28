// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagAssetInterface.h"

#include "Enums/UR_PickupState.h"
#include "UR_Pickup.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UCapsuleComponent;
class UParticleSystemComponent;
class UPrimitiveComponent;
class AUR_Character;
class UFXSystemAsset;

/////////////////////////////////////////////////////////////////////////////////////////////////

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPickedUpSignature, AUR_Pickup*, Pickup, APawn*, Recipient);

/**
 * Pickup Base Class - Actor representing a given Inventory class within the game world.
 *                     Pickups handle behavior such as whether pickup is allowed, timers
 *                     for respawning, etc.
 */
UCLASS(Abstract, Blueprintable)
class OPENTOURNAMENT_API AUR_Pickup : public AActor,
    public IGameplayTagAssetInterface
{
    GENERATED_BODY()
    
public:	

    AUR_Pickup(const FObjectInitializer& ObjectInitializer);
   
    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Collision Component of Pickup
    */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
    UCapsuleComponent* CollisionComponent;

    /**
    * Mesh Representation of Pickup
    */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
    UStaticMeshComponent* StaticMesh;

    /*
    * ParticleSystem Component
    */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
    UParticleSystemComponent* ParticleSystemComponent;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Event bound to Collision Overlaps
    */
    UFUNCTION()
    void OnOverlap(class UPrimitiveComponent* HitComp, class AActor* Other, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    /**
    * Is this Pickup event Valid?
    */
    bool IsPickupValid(const AUR_Character* PickupCharacter) const;

    /**
    * Is this Character permitted to pickup?
    */
    bool IsPickupPermitted(const AUR_Character* PickupCharacter) const;

    /**
    * Is this actor permitted to pickup given its associated GameplayTags?
    */
    UFUNCTION()
    bool IsPermittedByGameplayTags(const FGameplayTagContainer& TargetTags) const;

    /**
    * Actions to perform on Pickup
    */
    UFUNCTION()
    void Pickup(AUR_Character* PickupCharacter);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastPickedUp(AUR_Character* PickupCharacter);

    /**
    * Server & Client.
    * Actions to perform on pickup.
    * Return true to destroy the pickup, false to keep it alive.
    */
    UFUNCTION(BlueprintNativeEvent, Category = "Pickup")
    bool OnPickup(AUR_Character* PickupCharacter);

    /**
    * Client only. Play pickup effects/sounds.
    */
    UFUNCTION(BlueprintCosmetic, BlueprintNativeEvent, BlueprintCallable, Category = "Pickup")
    void PlayPickupEffects(AUR_Character* PickupCharacter);

    UPROPERTY(BlueprintAssignable)
    FOnPickedUpSignature OnPickedUp;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Sound played on Pickup
    */
    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Pickup")
    USoundBase* PickupSound;

    /**
    * Effect played on Pickup
    */
    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Pickup")
    UFXSystemAsset* PickupEffect;

    /**
    * Sound played on Respawn
    */
    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Pickup")
    USoundBase* RespawnSound;

    // @! TODO : FText for Localization?
    /**
    * Display Name of Pickup
    */
    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Pickup")
    FString DisplayName;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Gameplay Tags

    virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override { TagContainer = GameplayTags; }

    /**
    * Gameplay Tags for this Actor
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayTags")
    FGameplayTagContainer GameplayTags;

    /**
    * Are RequiredTags Exact?
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayTags")
    bool bRequiredTagsExact;

    /**
    * Actors attempting to Teleport must have at least one exact Tag match
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayTags")
    FGameplayTagContainer RequiredTags;

    /**
    * Are ExcludedTags Exact?
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayTags")
    bool bExcludedTagsExact;

    /**
    * Gameplay Tags for this Actor
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayTags")
    FGameplayTagContainer ExcludedTags;
};
