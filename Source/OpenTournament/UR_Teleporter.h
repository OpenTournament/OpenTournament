// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "GameFramework/Actor.h"

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
class UMaterialInstanceDynamic;
class UNavLinkComponent;

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
* - DestinationTransform should be invisible. Instead use a Component and bind delegate to Component location update to calculate DestinationTransform
* - Updating NavMesh and other interfacing with AI
*
* Visuals
* - Update Mesh to use simple Teleporter Mesh
* - Update Mesh to use MaterialInstance and setup Glow behaviors
*/
UCLASS(Abstract, BlueprintType, Blueprintable, HideCategories = (Actor, Rendering, Replication, Collision, Input, LOD, Cooking))
class OPENTOURNAMENT_API AUR_Teleporter : public AActor,
                                          public IGameplayTagAssetInterface
{
    GENERATED_BODY()

    /////////////////////////////////////////////////////////////////////////////////////////////////
private:
    /*
    * Static Mesh Component - Teleporter Base
    */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Teleporter", meta=(AllowPrivateAccess = "true"))
    UStaticMeshComponent* MeshComponent;

    /*
    * Capsule Component - Active Teleport Region
    */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Teleporter", meta=(AllowPrivateAccess = "true"))
    UCapsuleComponent* CapsuleComponent;

    /*
    * Audio Component
    */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Teleporter", meta=(AllowPrivateAccess = "true"))
    UAudioComponent* AudioComponent;

    /*
    * ParticleSystem Component
    */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Teleporter", meta=(AllowPrivateAccess = "true"))
    UParticleSystemComponent* ParticleSystemComponent;

    /*
    * Arrow Component
    */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Teleporter", meta=(AllowPrivateAccess = "true"))
    UArrowComponent* ArrowComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Teleporter", meta = (AllowPrivateAccess = "true"))
    UNavLinkComponent* NavLink;

    /////////////////////////////////////////////////////////////////////////////////////////////////
public:
    AUR_Teleporter(const FObjectInitializer& ObjectInitializer);

    virtual void OnConstruction(const FTransform& Transform) override;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Teleport Behavior

    /**
    * On Overlap with CollisionCapsule
    */
    UFUNCTION()
    void OnTriggerEnter(class UPrimitiveComponent* HitComp, class AActor* Other, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    /**
    * On End Overlap with CollisionCapsule
    */
    UFUNCTION()
    void OnTriggerExit(class UPrimitiveComponent* HitComp, class AActor* Other, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    /**
    * Try to Teleport the given Actor.
    * This may fail if preconditions are not met.
    */
    UFUNCTION(BlueprintCallable, Category = "Teleporter")
    void Teleport(AActor* Other);

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
    * Internal Teleport. Actually performs the Teleport.
    * Return true if successful.
    */
    bool InternalTeleport(AActor* TargetActor);

    /**
    * Get the DesiredRotation for the TargetActor
    */
    void GetDesiredRotation(FRotator& DesiredRotation, const FRotator& TargetActorRotation, const FRotator& DestinationRotation) const;

    /**
    * Set Teleport Target Actor's Velocity
    */
    void SetTargetVelocity(AActor* TargetActor, ACharacter* TargetCharacter, const FRotator& DesiredRotation, const FRotator& DestinationRotation);

    /**
    * Play Teleport Effects
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Teleporter")
    void PlayTeleportEffects();

    /**
    * Apply a GameplayTag to a Teleporting Actor
    */
    UFUNCTION()
    void ApplyGameplayTag(AActor* TargetActor);

    /**
    * Determine whether a given Actor is Ignored
    */
    UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Teleporter")
    bool IsIgnoredActor(AActor* InActor) const;

    /**
    * Set an Actor to be Ignored for the next single teleport.
    * Used to exclude an actor from teleporting
    */
    UFUNCTION()
    void AddIgnoredActor(AActor* InActor);

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Teleport Properties

    // @! TODO HIDE THIS - Instead use a Component and on updating the component position, update this value
    /**
    * Destination of Teleport - By Transform. Only editable when DestinationTeleporter is none.
    */
    UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "Teleporter", meta = (MakeEditWidget = ""))
    FTransform DestinationTransform;

    /**
    * Destination of Teleport - May be another Teleporter, TargetPoint, etc.
    */
    UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "Teleporter")
    AActor* DestinationActor;

    /**
    * Does our teleport force us into a new rotation, or is it relative to our DestinationActor's rotation?
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Teleporter")
    EExitRotation ExitRotationType;

    /**
    * Do Actors teleported retain their velocity?
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Teleporter")
    bool bKeepMomentum;

    /**
    * Actors of this ActorClass can interact with our Teleporter
    */
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Teleporter")
    TSubclassOf<AActor> TeleportActorClass;

    /**
    * Actors we Ignore the next single Teleport they engage in.
    * Used to exclude Teleportee from bouncing between connected Teleporters.
    */
    UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Teleporter|Debug")
    TArray<AActor*> IgnoredActors;

    /**
    * Tag Applied to Actor after Teleport
    */
    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Teleporter")
    FGameplayTag TeleportTag;

    /**
    * Sound when Actor teleports out
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleporter|Audio")
    USoundBase* TeleportOutSound;

    /**
    * Sound when Actor teleports in
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleporter|Audio")
    USoundBase* TeleportInSound;

    /**
    * Sound when Teleporter is Enabled/Activated
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleporter|Audio")
    USoundBase* TeleporterEnabledSound;

    /**
    * Sound when Teleporter is Disabled/Deactivated
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleporter|Audio")
    USoundBase* TeleporterDisabledSound;

    /**
    * ParticleSystem when Actor teleports out
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleporter|Effects")
    UParticleSystem* TeleportOutParticleSystemClass;

    /**
    * ParticleSystem when Actor teleports in
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleporter|Effects")
    UParticleSystem* TeleportInParticleSystemClass;

    /**
    * ParticleSystem when Teleporter is Enabled
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleporter|Effects")
    UParticleSystem* TeleporterEnabledParticleSystemClass;

    /**
    * ParticleSystem when Teleporter is Disabled
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleporter|Effects")
    UParticleSystem* TeleporterDisabledParticleSystemClass;


    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Teleporter State API

    /**
    * Set the Teleporter Enabled or Disabled
    */
    UFUNCTION(BlueprintCallable, Category = "Teleporter")
    void SetTeleporterState(const bool bInEnabled);

    /**
    * Internal. Activate the Teleporter and call our OnActivated event.
    */
    void Enable();

    /**
    * Event. Behavior  Teleporter and call our OnActivated event.
    */
    UFUNCTION(BlueprintNativeEvent, Category = "Teleporter")
    void OnEnabled();

    /**
    * Internal. Deactivate the Teleporter and call our OnDeactivated event
    */
    void Disable();

    /**
    * Event. Behavior  Teleporter and call our OnDeactivated event.
    */
    UFUNCTION(BlueprintNativeEvent, Category = "Teleporter")
    void OnDisabled();

    /**
    * Invoking this function will set this Teleporter to use the DestinationTransform.
    * Set the Teleport Destination Transform
    */
    UFUNCTION(BlueprintCallable, Category = "Teleporter")
    void SetTeleportDestination(const FTransform& InTransform);

    /**
    * Invoking this function will set this Teleporter to use the DestinationActor.
    * Set the Teleport Destination Actor.
    */
    UFUNCTION(BlueprintCallable, Category = "Teleporter")
    void SetTeleportDestinationActor(AActor* InActor);

    /**
    * Is the Teleporter Enabled?
    */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Teleporter")
    bool bIsEnabled;


    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Gameplay Tags

    virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override
    {
        TagContainer = GameplayTags;
    }

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
    // Dynamic MaterialInstance

    /**
    * Optional. Requires TeleporterMaterialInstance > -1.
    * Initialize a DynamicMaterialInstance for this Mesh to allow for per-instance Material changes.
    */
    UFUNCTION(BlueprintCallable, Category = "Teleporter|MaterialInstance")
    void InitializeDynamicMaterialInstance();

    /**
    * Index of Material used by the Mesh that will be used as the basis of our DynamicMaterialInstance
    */
    UPROPERTY(BlueprintReadWrite, Category = "Teleporter|MaterialInstance")
    UMaterialInstanceDynamic* TeleporterMaterialInstance;

    /**
    * Index of Material used by the Mesh
    */
    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Teleporter|MaterialInstance")
    int32 TeleporterMaterialIndex;

    /**
    * ParameterName of Material that will dynamically update
    */
    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Teleporter|MaterialInstance")
    FName TeleporterMaterialParameterName;

    /**
    * Active Color
    */
    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Teleporter|MaterialInstance")
    FLinearColor TeleporterMaterialColorBase;

    /**
    * 'Event' Color
    */
    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Teleporter|MaterialInstance")
    FLinearColor TeleporterMaterialColorEvent;

    /**
    * Inactive Color
    */
    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Teleporter|MaterialInstance")
    FLinearColor TeleporterMaterialColorInactive;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Conditional Edit Properties

#if WITH_EDITOR
    virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
};
