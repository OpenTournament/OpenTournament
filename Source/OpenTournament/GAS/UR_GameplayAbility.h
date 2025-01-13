// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Abilities/GameplayAbility.h"

#include "UR_AbilityTypes.h"
#include "UR_Ability_GameActivationGroup.h"
#include "UR_Ability_GameActivationPolicy.h"

#include "UR_GameplayAbility.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

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
    UTexture2D* AbilityIcon;

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
UCLASS(Abstract, HideCategories = Input, Meta = (ShortTooltip = "The base gameplay ability class used by this project."))
class OPENTOURNAMENT_API UUR_GameplayAbility : public UGameplayAbility
{
    GENERATED_BODY()

    friend class UUR_AbilitySystemComponent;

public:
    UUR_GameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    UFUNCTION(BlueprintCallable, Category = "Game|Ability")
    UUR_AbilitySystemComponent* GetGameAbilitySystemComponentFromActorInfo() const;

    UFUNCTION(BlueprintCallable, Category = "Game|Ability")
    AUR_PlayerController* GetGamePlayerControllerFromActorInfo() const;

    UFUNCTION(BlueprintCallable, Category = "Game|Ability")
    AController* GetControllerFromActorInfo() const;

    UFUNCTION(BlueprintCallable, Category = "Game|Ability")
    AUR_Character* GetGameCharacterFromActorInfo() const;

    UFUNCTION(BlueprintCallable, Category = "Game|Ability")
    UUR_HeroComponent* GetHeroComponentFromActorInfo() const;

    EGameAbilityActivationPolicy GetActivationPolicy() const
    {
        return ActivationPolicy;
    }

    EGameAbilityActivationGroup GetActivationGroup() const
    {
        return ActivationGroup;
    }

    void TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const;

    // Returns true if the requested activation group is a valid transition.
    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Game|Ability", Meta = (ExpandBoolAsExecs = "ReturnValue"))
    bool CanChangeActivationGroup(EGameAbilityActivationGroup NewGroup) const;

    // Tries to change the activation group.  Returns true if it successfully changed.
    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Game|Ability", Meta = (ExpandBoolAsExecs = "ReturnValue"))
    bool ChangeActivationGroup(EGameAbilityActivationGroup NewGroup);

    // Sets the ability's camera mode.
    UFUNCTION(BlueprintCallable, Category = "Game|Ability")
    void SetCameraMode(TSubclassOf<UUR_CameraMode> CameraMode);

    // Clears the ability's camera mode.  Automatically called if needed when the ability ends.
    UFUNCTION(BlueprintCallable, Category = "Game|Ability")
    void ClearCameraMode();

    void OnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const;

protected:
    // Called when the ability fails to activate
    virtual void NativeOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const;

    // Called when the ability fails to activate
    UFUNCTION(BlueprintImplementableEvent)
    void ScriptOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const;

#pragma region UGameplayAbilityInterface

    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
    virtual void SetCanBeCanceled(bool bCanBeCanceled) override;
    virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
    virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
    virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
    virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
    virtual FGameplayEffectContextHandle MakeEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const override;
    virtual void ApplyAbilityTagsToGameplayEffectSpec(FGameplayEffectSpec& Spec, FGameplayAbilitySpec* AbilitySpec) const override;
    virtual bool DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

#pragma endregion // UGameplayAbilityInterface

    virtual void OnPawnAvatarSet();

    virtual void GetAbilitySource(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, float& OutSourceLevel, const IUR_AbilitySourceInterface*& OutAbilitySource, AActor*& OutEffectCauser) const;

    /** Called when this ability is granted to the ability system component. */
    UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnAbilityAdded")
    void K2_OnAbilityAdded();

    /** Called when this ability is removed from the ability system component. */
    UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnAbilityRemoved")
    void K2_OnAbilityRemoved();

    /** Called when the ability system is initialized with a pawn avatar. */
    UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnPawnAvatarSet")
    void K2_OnPawnAvatarSet();

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

    /** Map of gameplay tags to gameplay effect containers */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GameplayEffects, meta = (DeprecatedProperty))
    TMap<FGameplayTag, FUR_GameplayEffectContainer> EffectContainerMap;

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

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability", meta = (DeprecatedProperty))
    FUR_GameplayAbilityData Data;

};
