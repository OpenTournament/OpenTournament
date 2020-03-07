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

/////////////////////////////////////////////////////////////////////////////////////////////////

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

    /**
    * BP-Implementable Hook Event for actions to perform on Pickup
    */
    UFUNCTION(BlueprintNativeEvent, Category = "Pickup")
    void OnPickup(AUR_Character* PickupCharacter);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Set the Pickup State
    */
    UFUNCTION(BlueprintCallable, Category = "Pickup")
    void SetPickupState(EPickupState NewState);

    /**
    * Respawn Pickup
    */
    UFUNCTION(Category = "Pickup")
    void RespawnPickup();

    /**
    * Pickup State
    */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Pickup")
    EPickupState PickupState;

    /**
    * Time (in Seconds) for an Inactive Pickup to become Active
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Pickup")
    float RespawnInterval;


    FTimerHandle RespawnHandle;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Sound played on Pickup
    */
    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Pickup")
    USoundBase* PickupSound;

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

    virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override { TagContainer = GameplayTags; return; }

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
