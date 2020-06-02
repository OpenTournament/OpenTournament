// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagAssetInterface.h"

#include "UR_TriggerZone.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations

class UPrimitiveComponent;
class UShapeComponent;

/////////////////////////////////////////////////////////////////////////////////////////////////
// Delegates

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActorEnter, AActor*, EnteringActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActorExit, AActor*, ExitingActor);

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
* Trigger Zone Base Class
* A Trigger Zone defines a level-designer specifiable space which tracks which actors (specifiable)
* are within a given space.
* Intentionally does very little in order to allow more modularity & extensibility.
*
* Typical usage of TriggerZone would be paired with a Control Point actor to create
* a Domination Point, as part of a King of the Hill gametype, etc.
*
* Complex spaces (e.g., a large rectangular 'Trigger Zone' with a hole cut out in the center)
* can be formed by creating BP subclasses utilizing additional shape components.
* Upon entering these sub-zones, use overlap events within the BP to add/remove actors from the tracked Actors.
* 
* This class is abstract.
* For placeable versions, see AUR_TriggerZone_Box & AUR_TriggerZone_Capsule.
* 
* @! TODO: Issues to Address
* - Characters Entering Zone by unusual means (Spawning, Teleporting)
* - Characters Leaving Zone by alternate means (Dying, Teleporting)
*/
UCLASS(Abstract, NotBlueprintable, HideCategories = (Tick, Rendering, Replication, Input, Actor, LOD, Cooking))
class OPENTOURNAMENT_API AUR_TriggerZone : public AActor,
    public IGameplayTagAssetInterface
{
    GENERATED_BODY()

    /////////////////////////////////////////////////////////////////////////////////////////////////
public:

    AUR_TriggerZone(const FObjectInitializer& ObjectInitializer);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    //virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /**
    * Override PostInitializeComponents to Bind ShapeComponent Events
    */
    virtual void PostInitializeComponents() override;

    /**
    * Check for Errors to find instances where this actor is configured incorrectly.
    */
    virtual void CheckForErrors();

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * ShapeComponent we will bind Events to
    */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TriggerZone")
    UShapeComponent* ShapeComponent;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Zone Enter

    /**
    * C++ Only. Handles the internals of ZoneEnter behavior.
    */
    void InternalZoneEnter(AActor* InActor);

    /**
    * On Begin Overlap with CollisionShape
    */
    UFUNCTION()
    void OnZoneEnter(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    /**
    * BP-Callable version of OnZoneEnter, for explicitly instigating a ZoneEnter
    */
    UFUNCTION(BlueprintCallable, Category = "TriggerZone")
    void ZoneEnter(AActor* InActor);

    /**
    * BP-Exposed Event to occur On Enter
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TriggerZone")
    void OnEnter(AActor* EnteringActor);

    /**
    * Event Binding / Delegate Hook for Events fired when an Actor Enters
    */
    UPROPERTY(BlueprintAssignable)
    FOnActorEnter OnActorEnter;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Zone Exit

    /**
    * C++ Only. Handles the internals of ZoneExit behavior.
    */
    void InternalZoneExit(AActor* InActor);

    /**
    * On End Overlap with CollisionShape
    */
    UFUNCTION()
    void OnZoneExit(UPrimitiveComponent* HitComp, class AActor* Other, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    /**
    * BP-Callable version of OnZoneExit, for explicitly instigating a ZoneExit
    */
    UFUNCTION(BlueprintCallable, Category = "TriggerZone")
    void ZoneExit(AActor* InActor);

    /**
    * BP-Exposed Event to occur On Exit
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TriggerZone")
    void OnExit(AActor* ExitingActor);

    /**
    * Event Binding / Delegate Hook for Events fired when an Actor Exits
    */
    UPROPERTY(BlueprintAssignable)
    FOnActorExit OnActorExit;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Triggering Actor

    /**
    * Is this Actor permitted to Trigger?
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintPure, BlueprintCallable, Category = "TriggerZone")
    bool IsTriggerActor(const AActor* InActor) const;

    /**
    * Is an Actor with these GameplayTags permitted to Trigger?
    */
    UFUNCTION()
    bool IsTriggerByGameplayTags(const FGameplayTagContainer& TargetTags) const;

    /**
    * Actors of this ActorClass can Trigger our Zone
    */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TriggerZone")
    TSubclassOf<AActor> TriggerActorClass;

    /**
    * Triggering Actors in the TriggerZone
    */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TriggerZone")
    TArray<AActor*> TriggerActors;

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
    * Actors attempting to Capture must have at least one exact Tag match
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
    virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
};
