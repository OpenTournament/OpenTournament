// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbility.h"

#include "UR_AbilityTypes.h"
#include "UR_Ability_GameActivationGroup.h"
#include "UR_Ability_GameActivationPolicy.h"

#include "UR_GameplayAbility.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UUR_AbilitySystemComponent;

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

    FUR_GameplayAbilityData()
        : AbilityIcon(nullptr)
    {
    }
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

    friend class UR_GameplayAbilitySystemComponent;

public:
    // Constructor and overrides
    UUR_GameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    //

    EGameAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }
    EGameAbilityActivationGroup GetActivationGroup() const { return ActivationGroup; }

    UFUNCTION(BlueprintCallable, Category = "OT|Ability")
    UUR_AbilitySystemComponent* GetGameAbilitySystemComponentFromActorInfo() const;

    // Returns true if the requested activation group is a valid transition.
    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "OT|Ability", Meta = (ExpandBoolAsExecs = "ReturnValue"))
    bool CanChangeActivationGroup(EGameAbilityActivationGroup NewGroup) const;

    // Tries to change the activation group.  Returns true if it successfully changed.
    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "OT|Ability", Meta = (ExpandBoolAsExecs = "ReturnValue"))
    bool ChangeActivationGroup(EGameAbilityActivationGroup NewGroup);


    //

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

    //
protected:

    // Defines how this ability is meant to activate.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Ability Activation")
    EGameAbilityActivationPolicy ActivationPolicy;

    // Defines the relationship between this ability activating and other abilities activating.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Ability Activation")
    EGameAbilityActivationGroup ActivationGroup;

};
