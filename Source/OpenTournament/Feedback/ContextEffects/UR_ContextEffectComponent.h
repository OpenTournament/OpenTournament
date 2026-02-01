// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Components/ActorComponent.h"
#include "UR_ContextEffectsInterface.h"

#include "UR_ContextEffectComponent.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

#define UE_API OPENTOURNAMENT_API

namespace EEndPlayReason
{
    enum Type : int;
}

class UAnimSequenceBase;
class UAudioComponent;
class UUR_ContextEffectsLibrary;
class UNiagaraComponent;
class UObject;
class USceneComponent;
struct FFrame;
struct FHitResult;

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(MinimalAPI, ClassGroup=(Custom), hidecategories = (Variable, Tags, ComponentTick, ComponentReplication, Activation, Cooking, AssetUserData, Collision), CollapseCategories, meta=(BlueprintSpawnableComponent))
class UUR_ContextEffectComponent : public UActorComponent, public IUR_ContextEffectsInterface
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UE_API UUR_ContextEffectComponent();

protected:
    // Called when the game starts
    UE_API virtual void BeginPlay() override;

    // Called when the game ends
    UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // AnimMotionEffect Implementation
    UFUNCTION(BlueprintCallable)
    UE_API virtual void AnimMotionEffect_Implementation(const FName Bone, const FGameplayTag MotionEffect, USceneComponent* StaticMeshComponent,
                                                        const FVector LocationOffset, const FRotator RotationOffset, const UAnimSequenceBase* AnimationSequence,
                                                        const bool bHitSuccess, const FHitResult HitResult, FGameplayTagContainer Contexts,
                                                        FVector VFXScale = FVector(1), float AudioVolume = 1, float AudioPitch = 1) override;

    // Auto-Convert Physical Surface from Trace Result to Context
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bConvertPhysicalSurfaceToContext = true;

    // Default Contexts
    UPROPERTY(EditAnywhere)
    FGameplayTagContainer DefaultEffectContexts;

    // Default Libraries for this Actor
    UPROPERTY(EditAnywhere)
    TSet<TSoftObjectPtr<UUR_ContextEffectsLibrary>> DefaultContextEffectsLibraries;

    UFUNCTION(BlueprintCallable)
    UE_API void UpdateEffectContexts(FGameplayTagContainer NewEffectContexts);

    UFUNCTION(BlueprintCallable)
    UE_API void UpdateLibraries(const TSet<TSoftObjectPtr<UUR_ContextEffectsLibrary>> NewContextEffectsLibraries);

private:
    UPROPERTY(Transient)
    FGameplayTagContainer CurrentContexts;

    UPROPERTY(Transient)
    TSet<TSoftObjectPtr<UUR_ContextEffectsLibrary>> CurrentContextEffectsLibraries;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UAudioComponent>> ActiveAudioComponents;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UNiagaraComponent>> ActiveNiagaraComponents;
};

#undef UE_API
