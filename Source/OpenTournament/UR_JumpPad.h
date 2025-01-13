// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "GameFramework/Actor.h"

#include "UR_JumpPad.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UAudioComponent;
class UCapsuleComponent;
class UMaterialInstanceDynamic;
class UParticleSystem;
class UParticleSystemComponent;
class USplineComponent;
class USoundBase;
class UStaticMeshComponent;
class UNavLinkComponent;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
* Jump Pad Base Class
* A JumpPad is an actor that imparts velocity to a given TargetActor such that they may
* reach a given destination in the world.
*
* Destination is a transform in the world that may be moved by editor widget.
*
* @! TODO: Issues to address:
* - API for changing JumpPad destination
* - Visualization for displaying (in-game) to nearby local clients the path of the JumpPad (use arc visualization?)
* - Expose API to permit adjustments to Character air control from JumpPad
* - Updating NavMesh and other interfacing with AI
*/
UCLASS(Blueprintable, Abstract, HideCategories = (Actor, Rendering, Replication, Collision, Input, LOD, Cooking))
class OPENTOURNAMENT_API AUR_JumpPad : public AActor,
                                       public IGameplayTagAssetInterface
{
    GENERATED_BODY()

    /////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /**
    * Root Component
    */
    UPROPERTY(BlueprintReadOnly, Category = JumpPad)
    USceneComponent* SceneRoot;

    /*
    * Static Mesh Component - JumpPad Base
    */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "JumpPad")
    UStaticMeshComponent* MeshComponent;

    /*
    * Capsule Component - Active JumpPad Region
    */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "JumpPad")
    UCapsuleComponent* CapsuleComponent;

    /*
    * Audio Component
    */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "JumpPad")
    UAudioComponent* AudioComponent;

    /*
    * ParticleSystem Component
    */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "JumpPad")
    UParticleSystemComponent* ParticleSystemComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "JumpPad")
    UNavLinkComponent* NavLink;

#if WITH_EDITOR
    /*
    * Spline Component
    */
    USplineComponent* SplineComponent;
#endif

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /*
    * Destination of the Jump - May be another JumpPad, TargetPoint, etc.
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpPad", meta = (MakeEditWidget = ""))
    FTransform Destination;

    /**
    * If set then lock the Destination when moving/rotating the JumpPad
    */
    UPROPERTY(EditAnywhere, Category = "JumpPad|Editor")
    bool bLockDestination;

    /**
    * Should players retain Horizontal (XY) Velocity when they hit this JumpPad?
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpPad")
    bool bRetainHorizontalVelocity;

    /**
    * Class of Actors capable of interacting with JumpPad
    */
    UPROPERTY(EditAnywhere, Category = "JumpPad")
    TSubclassOf<AActor> JumpActorClass;

    /*
    * Desired duration of the Jump (in seconds). Used to calculate velocity needed to impart
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "JumpPad")
    float JumpDuration;

    /**
    * Sound played on Launch
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpPad")
    USoundBase* JumpPadLaunchSound;

    /**
    * Particle System created on Launch
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpPad")
    UParticleSystem* JumpPadLaunchParticleClass;

    /**
    * Editor-only.
    * Spline Jump-Arc Projection will Calculate for this many Seconds
    */
    UPROPERTY(EditInstanceOnly, Category = "JumpPad|Editor", Meta=(UIMin = "0.0", UIMax ="10.0"))
    float SplineProjectionDuration;

    /////////////////////////////////////////////////////////////////////////////////////////////////

public:
    AUR_JumpPad(const FObjectInitializer& ObjectInitializer);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    virtual void OnConstruction(const FTransform& Transform) override;

    void BeginPlay() override;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Bound to Capsule Overlap Event.
    */
    UFUNCTION()
    void OnTriggerEnter(class UPrimitiveComponent* HitComp, class AActor* Other, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    /**
    * Is this actor permitted to jump?
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintPure, BlueprintCallable, Category = "JumpPad")
    bool IsPermittedToJump(const AActor* TargetActor) const;

    /**
    * Is this actor permitted to teleport given its associated GameplayTags?
    */
    UFUNCTION()
    bool IsPermittedByGameplayTags(const FGameplayTagContainer& TargetTags) const;

    /**
    * Calculate our Jump Velocity
    */
    FVector CalculateJumpVelocity(const AActor* InCharacter) const;

    /**
    * Play Effects on Successful Jump
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "JumpPad")
    void PlayJumpPadEffects();

    /**
    * Set the Destination of the JumpPad
    */
    UFUNCTION(BlueprintCallable, Category = "JumpPad")
    void SetDestination(const FVector InPosition, const bool IsRelativePosition = false);

#if WITH_EDITOR
    /**
    * Update Spline that visualizes the jump path
    */
    void UpdateSpline() const;
#endif

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Gameplay Tags

    virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override
    {
        TagContainer = GameplayTags;
        return;
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

    UFUNCTION()
    void InitializeDynamicMaterialInstance();

    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "JumpPad|MaterialInstance")
    bool bUseJumpPadMaterialInstance;

    UPROPERTY(BlueprintReadWrite, Category = "JumpPad|MaterialInstance")
    UMaterialInstanceDynamic* JumpPadMaterialInstance;

    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "JumpPad|MaterialInstance")
    int32 JumpPadMaterialIndex;

    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "JumpPad|MaterialInstance")
    FName JumpPadMaterialParameterName;

    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "JumpPad|MaterialInstance")
    FLinearColor JumpPadMaterialColorBase;

    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "JumpPad|MaterialInstance")
    FLinearColor JumpPadMaterialColorJump;

    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "JumpPad|MaterialInstance")
    FLinearColor JumpPadMaterialColorInactive;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Conditional Edit Properties

#if WITH_EDITOR
    virtual bool CanEditChange(const FProperty* InProperty) const override;
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    /////////////////////////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
    virtual void EditorApplyTranslation(const FVector& DeltaTranslation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
    virtual void EditorApplyRotation(const FRotator& DeltaRotation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
    virtual void EditorApplyScale(const FVector& DeltaScale, const FVector* PivotLocation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
#endif
};
