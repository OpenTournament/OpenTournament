// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "AbilitySystemComponent.h"
#include "UR_Ability_GameActivationGroup.h"

#include <NativeGameplayTags.h>

#include "UR_Ability_GameActivationGroup.h"

#include "UR_AbilitySystemComponent.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UUR_GameplayAbility;
class UUR_AbilityTagRelationshipMapping;

/////////////////////////////////////////////////////////////////////////////////////////////////

OPENTOURNAMENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_AbilityInputBlocked);

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Subclass of ability system component with game-specific data
 * Game-specific subclass to provide utility functions
 */
UCLASS()
class OPENTOURNAMENT_API UUR_AbilitySystemComponent : public UAbilitySystemComponent
{
    GENERATED_BODY()

public:

    UUR_AbilitySystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

#pragma region UActorComponentInterface

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#pragma endregion // UActorComponentInterface

    virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;

    using TShouldCancelAbilityFunc = TFunctionRef<bool(const UUR_GameplayAbility* InGameAbility, FGameplayAbilitySpecHandle Handle)>;
    void CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc, bool bReplicateCancelAbility);

    void CancelInputActivatedAbilities(bool bReplicateCancelAbility);

    void AbilityInputTagPressed(const FGameplayTag& InputTag);
    void AbilityInputTagReleased(const FGameplayTag& InputTag);

    void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
    void ClearAbilityInput();

    bool IsActivationGroupBlocked(EGameAbilityActivationGroup InGroup) const;
    void AddAbilityToActivationGroup(EGameAbilityActivationGroup InGroup, UUR_GameplayAbility* InGameAbility);
    void RemoveAbilityFromActivationGroup(EGameAbilityActivationGroup InGroup, UUR_GameplayAbility* InAbility);
    void CancelActivationGroupAbilities(EGameAbilityActivationGroup Group, UUR_GameplayAbility* IgnoreGameAbility, bool bReplicateCancelAbility);

    // Uses a gameplay effect to add the specified dynamic granted tag.
    void AddDynamicTagGameplayEffect(const FGameplayTag& Tag);

    // Removes all active instances of the gameplay effect that was used to add the specified dynamic granted tag.
    void RemoveDynamicTagGameplayEffect(const FGameplayTag& Tag);

    /** Gets the ability target data associated with the given ability handle and activation info */
    void GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle, FGameplayAbilityActivationInfo ActivationInfo, FGameplayAbilityTargetDataHandle& OutTargetDataHandle);

    /** Sets the current tag relationship mapping, if null it will clear it out */
    void SetTagRelationshipMapping(UUR_AbilityTagRelationshipMapping* NewMapping);

    /** Looks at ability tags and gathers additional required and blocking tags */
    void GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer& OutActivationRequired, FGameplayTagContainer& OutActivationBlocked) const;


    /////////////////////////////////////////////////////////////////////////////////////////////////
protected:

    void TryActivateAbilitiesOnSpawn();

    virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
    virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;

    virtual void NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability) override;
    virtual void NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason) override;
    virtual void NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled) override;
    virtual void ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags) override;
    virtual void HandleChangeAbilityCanBeCanceled(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bCanBeCanceled) override;

    /** Notify client that an ability failed to activate */
    UFUNCTION(Client, Unreliable)
    void ClientNotifyAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);

    void HandleAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);

    /////////////////////////////////////////////////////////////////////////////////////////////////
protected:

    // If set, this table is used to look up tag relationships for activate and cancel
    UPROPERTY()
    TObjectPtr<UUR_AbilityTagRelationshipMapping> TagRelationshipMapping;

    // Handles to abilities that had their input pressed this frame.
    TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

    // Handles to abilities that had their input released this frame.
    TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

    // Handles to abilities that have their input held.
    TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;

    // Number of abilities running in each activation group.
    int32 ActivationGroupCounts[static_cast<uint8>(EGameAbilityActivationGroup::MAX)];

    /////////////////////////////////////////////////////////////////////////////////////////////////
};
