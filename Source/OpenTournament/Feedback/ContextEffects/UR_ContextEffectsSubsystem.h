// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Subsystems/WorldSubsystem.h"

#include "GameplayTagContainer.h"

//#include "Feedback/ContextEffects/UR_ContextEffectsSettings.h"

#include "UR_ContextEffectsSubsystem.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

#define UE_API OPENTOURNAMENT_API

enum EPhysicalSurface : int;

class AActor;
class UAudioComponent;
class UUR_ContextEffectsLibrary;
class UNiagaraComponent;
class USceneComponent;
struct FFrame;
struct FGameplayTag;
struct FGameplayTagContainer;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
UCLASS(MinimalAPI)
class UUR_ContextEffectsSet : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(Transient)
    TSet<TObjectPtr<UUR_ContextEffectsLibrary>> ContextEffectsLibraries;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
UCLASS(MinimalAPI)
class UUR_ContextEffectsSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    /** */
    UFUNCTION(BlueprintCallable, Category = "ContextEffects")
    UE_API void SpawnContextEffects(
        const AActor* SpawningActor
        , USceneComponent* AttachToComponent
        , const FName AttachPoint
        , const FVector LocationOffset
        , const FRotator RotationOffset
        , FGameplayTag Effect
        , FGameplayTagContainer Contexts
        , TArray<UAudioComponent*>& AudioOut
        , TArray<UNiagaraComponent*>& NiagaraOut
        , FVector VFXScale = FVector(1)
        , float AudioVolume = 1
        , float AudioPitch = 1);

    /** */
    UFUNCTION(BlueprintCallable, Category = "ContextEffects")
    UE_API bool GetContextFromSurfaceType(TEnumAsByte<EPhysicalSurface> PhysicalSurface, FGameplayTag& Context);

    /** */
    UFUNCTION(BlueprintCallable, Category = "ContextEffects")
    UE_API void LoadAndAddContextEffectsLibraries(AActor* OwningActor, TSet<TSoftObjectPtr<UUR_ContextEffectsLibrary>> ContextEffectsLibraries);

    /** */
    UFUNCTION(BlueprintCallable, Category = "ContextEffects")
    UE_API void UnloadAndRemoveContextEffectsLibraries(AActor* OwningActor);

private:
    UPROPERTY(Transient)
    TMap<TObjectPtr<AActor>, TObjectPtr<UUR_ContextEffectsSet>> ActiveActorEffectsMap;
};

#undef UE_API
