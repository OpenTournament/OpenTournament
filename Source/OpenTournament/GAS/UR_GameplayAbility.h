// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Abilities/GameplayAbility.h"

#include "UR_AbilityTypes.h"
#include "UR_Ability_GameActivationGroup.h"
#include "UR_Ability_GameActivationPolicy.h"

#include "UR_GameplayAbility.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

#define UE_API OPENTOURNAMENT_API

class AActor;
class AController;
class AUR_PlayerController;
class AUR_Character;
class FText;
class IUR_AbilitySourceInterface;
class UObject;
class UUR_AbilityCost;
class UUR_AbilitySystemComponent;
class UUR_CameraMode;
class UUR_HeroComponent;

struct FFrame;
struct FGameplayAbilityActivationInfo;
struct FGameplayAbilityActorInfo;
struct FGameplayAbilitySpec;
struct FGameplayAbilitySpecHandle;
struct FGameplayEffectSpec;
struct FGameplayEventData;

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
    TObjectPtr<UTexture2D> AbilityIcon;

    FUR_GameplayAbilityData()
        : AbilityIcon(nullptr)
    {}
};

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Subclass of ability blueprint type with game-specific data
 * This class uses GameplayEffectContainers to allow easier execution of gameplay effects based on a triggering tag
 * Most games will need to implement a subclass to support their game-specific code
 */
UCLASS(MinimalAPI, Abstract, HideCategories = Input, Meta = (ShortTooltip = "The base gameplay ability class used by this project."))
class UUR_GameplayAbility : public UGameplayAbility
{
    GENERATED_BODY()

    friend class UUR_AbilitySystemComponent;

public:
    UUR_GameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    UFUNCTION(BlueprintCallable, Category = "Game|Ability")
    UE_API UUR_AbilitySystemComponent* GetGameAbilitySystemComponentFromActorInfo() const;

    UFUNCTION(BlueprintCallable, Category = "Game|Ability")
    UE_API AUR_PlayerController* GetGamePlayerControllerFromActorInfo() const;

    UFUNCTION(BlueprintCallable, Category = "Game|Ability")
    UE_API AController* GetControllerFromActorInfo() const;

    UFUNCTION(BlueprintCallable, Category = "Game|Ability")
    UE_API AUR_Character* GetGameCharacterFromActorInfo() const;

    UFUNCTION(BlueprintCallable, Category = "Game|Ability")
    UE_API UUR_HeroComponent* GetHeroComponentFromActorInfo() const;

    EGameAbilityActivationPolicy GetActivationPolicy() const
    {
        return ActivationPolicy;
    }

    EGameAbilityActivationGroup GetActivationGroup() const
    {
        return ActivationGroup;
    }

    UE_API void TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const;

    // Returns true if the requested activation group is a valid transition.
    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Game|Ability", Meta = (ExpandBoolAsExecs = "ReturnValue"))
    UE_API bool CanChangeActivationGroup(EGameAbilityActivationGroup NewGroup) const;

    // Tries to change the activation group.  Returns true if it successfully changed.
    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Game|Ability", Meta = (ExpandBoolAsExecs = "ReturnValue"))
    UE_API bool ChangeActivationGroup(EGameAbilityActivationGroup NewGroup);

    // Sets the ability's camera mode.
    UFUNCTION(BlueprintCallable, Category = "Game|Ability")
    UE_API void SetCameraMode(TSubclassOf<UUR_CameraMode> CameraMode);

    // Clears the ability's camera mode.  Automatically called if needed when the ability ends.
    UFUNCTION(BlueprintCallable, Category = "Game|Ability")
    UE_API void ClearCameraMode();

    void OnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const;

    UE_API TArray<FAbilityTriggerData> GetAbilityTriggers() const;

protected:
    // Called when the ability fails to activate
    UE_API virtual void NativeOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const;

    // Called when the ability fails to activate
    UFUNCTION(BlueprintImplementableEvent)
    UE_API void ScriptOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const;

#pragma region UGameplayAbilityInterface
    UE_API virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
    UE_API virtual void SetCanBeCanceled(bool bCanBeCanceled) override;
    UE_API virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
    UE_API virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
    UE_API virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    UE_API virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
    UE_API virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
    UE_API virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
    UE_API virtual FGameplayEffectContextHandle MakeEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const override;
    UE_API virtual void ApplyAbilityTagsToGameplayEffectSpec(FGameplayEffectSpec& Spec, FGameplayAbilitySpec* AbilitySpec) const override;
    UE_API virtual bool DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
#pragma endregion // UGameplayAbilityInterface

    UE_API virtual void OnPawnAvatarSet();

    UE_API virtual void GetAbilitySource(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, float& OutSourceLevel, const IUR_AbilitySourceInterface*& OutAbilitySource, AActor*& OutEffectCauser) const;

    /** Called when this ability is granted to the ability system component. */
    UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnAbilityAdded")
    UE_API void K2_OnAbilityAdded();

    /** Called when this ability is removed from the ability system component. */
    UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnAbilityRemoved")
    UE_API void K2_OnAbilityRemoved();

    /** Called when the ability system is initialized with a pawn avatar. */
    UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnPawnAvatarSet")
    UE_API void K2_OnPawnAvatarSet();

    /////////////////////////////////////////////////////////////////////////////////////////////////
protected:
    // Defines how this ability is meant to activate.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game|Ability Activation")
    EGameAbilityActivationPolicy ActivationPolicy;

    // Defines the relationship between this ability activating and other abilities activating.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game|Ability Activation")
    EGameAbilityActivationGroup ActivationGroup;

    // Additional costs that must be paid to activate this ability
    UPROPERTY(EditDefaultsOnly, Instanced, Category = Costs)
    TArray<TObjectPtr<UUR_AbilityCost>> AdditionalCosts;

    // Map of failure tags to simple error messages
    UPROPERTY(EditDefaultsOnly, Category = "Advanced")
    TMap<FGameplayTag, FText> FailureTagToUserFacingMessages;

    // Map of failure tags to anim montages that should be played with them
    UPROPERTY(EditDefaultsOnly, Category = "Advanced")
    TMap<FGameplayTag, TObjectPtr<UAnimMontage>> FailureTagToAnimMontage;

    // If true, extra information should be logged when this ability is canceled. This is temporary, used for tracking a bug.
    UPROPERTY(EditDefaultsOnly, Category = "Advanced")
    bool bLogCancellation;

    // Current camera mode set by the ability.
    TSubclassOf<UUR_CameraMode> ActiveCameraMode;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /** Make gameplay effect container spec to be applied later, using the passed in container */
    UFUNCTION(BlueprintCallable, Category = "Ability", meta=(AutoCreateRefTerm = "EventData", DeprecatedFunction))
    virtual FUR_GameplayEffectContainerSpec MakeEffectContainerSpecFromContainer(const FUR_GameplayEffectContainer& Container, const FGameplayEventData& EventData, int32 OverrideGameplayLevel = -1);

    /** Search for and make a gameplay effect container spec to be applied later, from the EffectContainerMap */
    UFUNCTION(BlueprintCallable, Category = "Ability", meta = (AutoCreateRefTerm = "EventData", DeprecatedFunction))
    virtual FUR_GameplayEffectContainerSpec MakeEffectContainerSpec(FGameplayTag ContainerTag, const FGameplayEventData& EventData, int32 OverrideGameplayLevel = -1);

    /** Applies a gameplay effect container spec that was previously created */
    UFUNCTION(BlueprintCallable, Category = "Ability", meta = (DeprecatedFunction))
    virtual TArray<FActiveGameplayEffectHandle> ApplyEffectContainerSpec(const FUR_GameplayEffectContainerSpec& ContainerSpec);

    /** Applies a gameplay effect container, by creating and then applying the spec */
    UFUNCTION(BlueprintCallable, Category = "Ability", meta = (AutoCreateRefTerm = "EventData", DeprecatedFunction))
    virtual TArray<FActiveGameplayEffectHandle> ApplyEffectContainer(FGameplayTag ContainerTag, const FGameplayEventData& EventData, int32 OverrideGameplayLevel = -1);
};

#undef UE_API
