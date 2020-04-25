// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagAssetInterface.h"

#include "UR_JumpPad.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UAudioComponent;
class UCapsuleComponent;
class UMaterialInstanceDynamic;
class UParticleSystem;
class UParticleSystemComponent;
class USoundBase;
class UStaticMeshComponent;

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(HideCategories = (Actor, Rendering, Replication, Collision, Input, LOD, Cooking))
class OPENTOURNAMENT_API AUR_JumpPad : public AActor,
    public IGameplayTagAssetInterface
{
    GENERATED_BODY()

    /////////////////////////////////////////////////////////////////////////////////////////////////
public:

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

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /*
    * Destination of the Jump - May be another JumpPad, TargetPoint, etc.
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpPad", meta = (MakeEditWidget = ""))
    FTransform Destination;

    /**
    * If set then lock the Destination when moving/rotating the JumpPad
    */
    UPROPERTY(EditAnywhere, Category = "JumpPadEditor")
    bool bLockDestination;

    /*
    * Duration of the Jump - in seconds
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "JumpPad")
    float JumpTime;

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

    /////////////////////////////////////////////////////////////////////////////////////////////////

public:

    AUR_JumpPad(const FObjectInitializer& ObjectInitializer);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    void BeginPlay() override;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Play Effects on Successful Jump
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "JumpPad")
    void PlayJumpPadEffects();

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

    UFUNCTION()
    void OnTriggerEnter(class UPrimitiveComponent* HitComp, class AActor* Other, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

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
    virtual bool CanEditChange(const UProperty* InProperty) const override;
#endif

    /////////////////////////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
    virtual void EditorApplyTranslation(const FVector& DeltaTranslation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
    virtual void EditorApplyRotation(const FRotator& DeltaRotation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
    virtual void EditorApplyScale(const FVector& DeltaScale, const FVector* PivotLocation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
#endif
};
