// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameFramework/Actor.h"
#include "GameplayTagAssetInterface.h"

#include "Enums/UR_ControlPointEnumerations.h"

#include "UR_ControlPoint.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class ACharacter;
class UChildActorComponent;
class AUR_TriggerZone;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
* Control Point
* A Control Point is a base class for a category map or mode specific objective actors
* Ex. Domination Point, Onslaught Node, King of the Hill Point, Assault Hold-to-Capture Objectives, etc.
*/
UCLASS(Abstract, BlueprintType, Blueprintable, HideCategories = (Tick, Rendering, Replication, Input, Actor, LOD, Cooking))
class OPENTOURNAMENT_API AUR_ControlPoint : public AActor,
    public IGameplayTagAssetInterface
{
    GENERATED_BODY()

    /////////////////////////////////////////////////////////////////////////////////////////////////
public:

    /**
    * TriggerZone ChildActor
    */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ControlPoint")
    UChildActorComponent* TriggerZoneComponent;

    /**
    * TriggerZone Reference
    */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ControlPoint")
    AUR_TriggerZone* TriggerZone;

    /**
    * ActorClass of TriggerZone
    */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ControlPoint")
    TSubclassOf<AUR_TriggerZone> TriggerZoneClass;

    /**
    * TriggerZone's Trigger ActorClass
    */
    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "ControlPoint")
    TSubclassOf<AActor> TriggerZoneActorClass;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    AUR_ControlPoint(const FObjectInitializer& ObjectInitializer);

    /**
    * Find our ShapeComponent if we don't have it set, and set up Event Bindings
    */
    virtual void PostInitializeComponents() override;

    /**
    * Actor Entered the TriggerZone
    */
    UFUNCTION(BlueprintCallable, Category = "ControlPoint")
    void ActorEnter(AActor* InActor);

    /**
    * On Overlap with TriggerZone
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ControlPoint")
    void OnActorEnter(AActor* InActor);

    /**
    * Actor Exited the TriggerZone
    */
    UFUNCTION(BlueprintCallable, Category = "ControlPoint")
    void ActorExit(AActor* InActor);

    /**
    * On Overlap with TriggerZone
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ControlPoint")
    void OnActorExit(AActor* InActor);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Should the Point be Contested?
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ControlPoint")
    bool ShouldPointBeContested(EControlPointEvent EventType, const AActor* TargetActor) const;

    /**
    * Set the Point's Contested State
    */
    UFUNCTION(BlueprintCallable, Category = "ControlPoint")
    void SetPointContestedState(bool bShouldBeContested, AActor* InActor = nullptr);

    /**
    * Is the Point Contested?
    */
    UFUNCTION(BlueprintPure, BlueprintCallable, Category = "ControlPoint")
    bool IsPointContested() const;

    /**
    * Event when the Point becomes Contested
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ControlPoint")
    void OnPointContested(AActor* InActor);

    /**
    * Event when the Point becomes Uncontested
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ControlPoint")
    void OnPointUncontested(AActor* InActor);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Is this actor permitted to Control the Point? 
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintPure, BlueprintCallable, Category = "ControlPoint")
    bool IsPermittedToControl(const AActor* TargetActor) const;

    /**
    * Set the Point's Control State
    */
    UFUNCTION(BlueprintCallable, Category = "ControlPoint")
    void SetPointControlState(bool bShouldBeControlled, AActor* InActor);

    /**
    * Event when the Point becomes Controlled
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ControlPoint")
    void OnPointControlled(AActor* InActor);

    /**
    * Event when the Point becomes UnControlled
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ControlPoint")
    void OnPointUncontrolled(AActor* InActor);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Is the Control Point's Zone occupied at all?
    */
    UFUNCTION(BlueprintPure, BlueprintCallable, Category = "ControlPoint")
    bool IsZoneOccupied() const;

    /**
    * Is the given Actor already considered in the Zone?
    */
    UFUNCTION(BlueprintPure, BlueprintCallable, Category = "ControlPoint")
    bool IsActorInZone(AActor* InActor) const;

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

    /**
    * Is this actor permitted to control or contest given its associated GameplayTags?
    */
    UFUNCTION()
    bool IsPermittedByGameplayTags(const FGameplayTagContainer& TargetTags) const;

    /**
    * Get the ActorControlTag - Such as Team or other Tag
    */
    UFUNCTION(BlueprintPure, BlueprintCallable, Category = "ControlPoint")
    FGameplayTag GetActorControlTag(const AActor* InActor) const;

    /**
    * Tag Applied to this Point when Contested
    */
    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "GameplayTags")
    FGameplayTag ContestedTag;

    /**
    * Tag Applied to this Point when Controlled
    */
    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "GameplayTags")
    FGameplayTag ControlTag;

    /**
    * Tag Applied to this Point when Controlled, taken from the Actor
    */
    UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "GameplayTags")
    FGameplayTag ActorControlTag;

    /**
    * Base of Tags that are taken from Actors for the ActorControlTag
    * e.g. Tag of format <Some>.<Thing>.Team will allow tags such as
    *      <Some>.<Thing>.Team.Team1 or <Some>.<Thing>.Team.Team2
    */
    UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "GameplayTags")
    FGameplayTag ActorControlTagBase;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Conditional Edit Properties

#if WITH_EDITOR
    virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
};
