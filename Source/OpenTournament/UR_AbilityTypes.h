// Copyright 2019 - 2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// ----------------------------------------------------------------------------------------------------------------
// This header is for Ability-specific structures and enums that are shared across a project
// Every game will probably need a file like this to handle their extensions to the system
// This file is a good place for subclasses of FGameplayEffectContext and FGameplayAbilityTargetData
// ----------------------------------------------------------------------------------------------------------------

#include "GameplayEffectTypes.h"
#include "GameplayAbilityTargetTypes.h"

#include "UR_AbilityTypes.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UGameplayEffect;
class UUR_TargetType;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Struct defining a list of gameplay effects, a tag, and targeting info
 * These containers are defined statically in blueprints or assets and then turn into Specs at runtime
 */
USTRUCT(BlueprintType)
struct FUR_GameplayEffectContainer
{
    GENERATED_BODY()

public:
    FUR_GameplayEffectContainer() {}

    /** Sets the way that targeting happens */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffectContainer)
    TSubclassOf<UUR_TargetType> TargetType;

    /** List of gameplay effects to apply to the targets */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffectContainer)
    TArray<TSubclassOf<UGameplayEffect>> TargetGameplayEffectClasses;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

/** A "processed" version of RPGGameplayEffectContainer that can be passed around and eventually applied */
USTRUCT(BlueprintType)
struct FUR_GameplayEffectContainerSpec
{
    GENERATED_BODY()

public:
    FUR_GameplayEffectContainerSpec() {}

    /** Computed target data */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffectContainer)
    FGameplayAbilityTargetDataHandle TargetData;

    /** List of gameplay effects to apply to the targets */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffectContainer)
    TArray<FGameplayEffectSpecHandle> TargetGameplayEffectSpecs;

    /** Returns true if this has any valid effect specs */
    bool HasValidEffects() const;

    /** Returns true if this has any valid targets */
    bool HasValidTargets() const;

    /** Adds new targets to target data */
    void AddTargets(const TArray<FHitResult>& HitResults, const TArray<AActor*>& TargetActors);
};
