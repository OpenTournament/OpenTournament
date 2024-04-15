// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"

#include "UR_AbilityTypes.h"

#include "UR_GameplayAbility.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FUR_GameplayAbilityData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "AbilityData")
    FText DisplayName;

    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "AbilityData")
    FText Description;

    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "AbilityData")
    UTexture2D* AbilityIcon;

    FUR_GameplayAbilityData() : AbilityIcon(NULL) {}
};

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Subclass of ability blueprint type with game-specific data
 * This class uses GameplayEffectContainers to allow easier execution of gameplay effects based on a triggering tag
 * Most games will need to implement a subclass to support their game-specific code
 */
UCLASS()
class OPENTOURNAMENT_API UUR_GameplayAbility : public UGameplayAbility
{
    GENERATED_BODY()

public:
    // Constructor and overrides
    UUR_GameplayAbility();

    /** Map of gameplay tags to gameplay effect containers */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GameplayEffects)
    TMap<FGameplayTag, FUR_GameplayEffectContainer> EffectContainerMap;

    /** Make gameplay effect container spec to be applied later, using the passed in container */
    UFUNCTION(BlueprintCallable, Category = "Ability", meta=(AutoCreateRefTerm = "EventData"))
    virtual FUR_GameplayEffectContainerSpec MakeEffectContainerSpecFromContainer(const FUR_GameplayEffectContainer& Container, const FGameplayEventData& EventData, int32 OverrideGameplayLevel = -1);

    /** Search for and make a gameplay effect container spec to be applied later, from the EffectContainerMap */
    UFUNCTION(BlueprintCallable, Category = "Ability", meta = (AutoCreateRefTerm = "EventData"))
    virtual FUR_GameplayEffectContainerSpec MakeEffectContainerSpec(FGameplayTag ContainerTag, const FGameplayEventData& EventData, int32 OverrideGameplayLevel = -1);

    /** Applies a gameplay effect container spec that was previously created */
    UFUNCTION(BlueprintCallable, Category = "Ability")
    virtual TArray<FActiveGameplayEffectHandle> ApplyEffectContainerSpec(const FUR_GameplayEffectContainerSpec& ContainerSpec);

    /** Applies a gameplay effect container, by creating and then applying the spec */
    UFUNCTION(BlueprintCallable, Category = "Ability", meta = (AutoCreateRefTerm = "EventData"))
    virtual TArray<FActiveGameplayEffectHandle> ApplyEffectContainer(FGameplayTag ContainerTag, const FGameplayEventData& EventData, int32 OverrideGameplayLevel = -1);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability")
    FUR_GameplayAbilityData Data;
};
