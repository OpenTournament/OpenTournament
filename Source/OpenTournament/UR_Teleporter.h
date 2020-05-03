// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagAssetInterface.h"

#include "UR_Type_ExitRotation.h"

#include "UR_Teleporter.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class ACharacter;
class UArrowComponent;
class UAudioComponent;
class UCapsuleComponent;
class USoundBase;
class UStaticMeshComponent;
class UParticleSystem;
class UParticleSystemComponent;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
* Teleporter Base Class
* A Teleporter is an actor that teleports a given TargetActor to a destination in the world.
*
* Destination may be a point in space or another actor (typically another Teleporter).
* The TargetActor's rotation may be transformed according to the facing of the destination
* Or preserved in world space.
* 
* @! TODO: Issues to address:
* - Class-specific occlusion in IsPermittedToTeleport
* - API for changing Teleporter destination (?)
* - API for enabling/disabling Teleporter (including visuals & effects)
* - Updating NavMesh and other interfacing with AI
* - MaterialInstance for teleporter base so base material can exhibit behaviors on teleportation (see JumpPad)
*/
UCLASS(HideCategories = (Actor, Rendering, Replication, Collision, Input, LOD, Cooking))
class OPENTOURNAMENT_API AUR_Teleporter : public AActor,
    public IGameplayTagAssetInterface
{
    GENERATED_BODY()

    /////////////////////////////////////////////////////////////////////////////////////////////////
public:

    /*
    * Static Mesh Component - Teleporter Base
    */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Teleporter")
    UStaticMeshComponent* MeshComponent;

    /*
    * Capsule Component - Active Teleport Region
    */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Teleporter")
    UCapsuleComponent* CapsuleComponent;

    /*
    * Audio Component
    */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Teleporter")
    UAudioComponent* AudioComponent;

    /*
    * Arrow Component
    */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Teleporter")
    UArrowComponent* ArrowComponent;

    /*
    * ParticleSystem Component
    */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Teleporter")
    UParticleSystemComponent* ParticleSystemComponent;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    AUR_Teleporter(const FObjectInitializer& ObjectInitializer);

    /**
    * On Overlap with CollisionCapsule
    */
    UFUNCTION()
    void OnTriggerEnter(class UPrimitiveComponent* HitComp, class AActor* Other, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    /**
    * Is this actor permitted to teleport? 
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintPure, BlueprintCallable, Category = "Teleporter")
    bool IsPermittedToTeleport(const AActor* TargetActor) const;

    /**
    * Is this actor permitted to teleport given its associated GameplayTags?
    */
    UFUNCTION()
    bool IsPermittedByGameplayTags(const FGameplayTagContainer& TargetTags) const;

    /**
    * Perform the teleport. Return true if successful.
    */
    UFUNCTION(BlueprintCallable)
    bool PerformTeleport(AActor* TargetActor);

    /**
    * Set Teleport Target Actor's Rotation
    */
    void SetTargetRotation(AActor* TargetActor, ACharacter* TargetCharacter, const FRotator& DesiredRotation);

    /**
    * Set Teleport Target Actor's Velocity
    */
    void SetTargetVelocity(AActor* TargetActor, ACharacter* TargetCharacter, const FRotator& DesiredRotation, const FRotator& DestinationRotation);

    /**
    * Apply a GameplayTag to a Teleporting Actor
    */
    UFUNCTION()
    void ApplyGameplayTag(AActor * TargetActor);

    /**
    * Play Teleport Effects
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Teleporter")
    void PlayTeleportEffects();


    void GetDesiredRotation(FRotator& DesiredRotation, const FRotator& TargetActorRotation, const FRotator& DestinationRotation);

    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Teleporter")
    FGameplayTag TeleportTag;


    /////////////////////////////////////////////////////////////////////////////////////////////////

    /*
    * Destination of Teleport - By Transform. Only editable when DestinationTeleporter is none.
    */
    UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "Teleporter", meta = (MakeEditWidget = ""))
    FTransform DestinationTransform;

    /*
    * Destination of Teleport - May be another Teleporter, TargetPoint, etc.
    */
    UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "Teleporter")
    AActor* DestinationActor;

    /*
    * Does our teleport force us into a new rotation, or is it relative to our DestinationActor's rotation?
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Teleporter")
    EExitRotation ExitRotationType;

    /*
    * Do Actors teleported retain their velocity?
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Teleporter")
    bool bKeepMomentum;

    /**
    * Sound when Actor teleports out
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleporter")
    USoundBase* TeleportOutSound;

    /**
    * Sound when Actor teleports in
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleporter")
    USoundBase* TeleportInSound;

    /**
    * ParticleSystem when Actor teleports out
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleporter")
    UParticleSystem* TeleportOutParticleSystemClass;

    /**
    * ParticleSystem when Actor teleports out
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleporter")
    UParticleSystem* TeleportInParticleSystemClass;

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

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Conditional Edit Properties

#if WITH_EDITOR
    virtual bool CanEditChange(const UProperty* InProperty) const override;
#endif
};