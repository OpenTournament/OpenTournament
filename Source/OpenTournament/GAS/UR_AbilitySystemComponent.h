// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "AbilitySystemComponent.h"
#include "UR_Ability_GameActivationGroup.h"

#include <NativeGameplayTags.h>

#include "UR_AbilitySystemComponent.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

#define UE_API OPENTOURNAMENT_API

class UUR_GameplayAbility;
class UUR_AbilityTagRelationshipMapping;

/////////////////////////////////////////////////////////////////////////////////////////////////

UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_AbilityInputBlocked);

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Subclass of ability system component with game-specific data
 * Game-specific subclass to provide utility functions
 */
UCLASS(MinimalAPI)
class UUR_AbilitySystemComponent : public UAbilitySystemComponent
{
    GENERATED_BODY()

public:
    // ReSharper disable CppUE4CodingStandardNamingViolationWarning
    using TShouldCancelAbilityFunc = TFunctionRef<bool(const UUR_GameplayAbility* InGameAbility, FGameplayAbilitySpecHandle Handle)>;
    // ReSharper restore CppUE4CodingStandardNamingViolationWarning

    UE_API UUR_AbilitySystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

#pragma region UActorComponentInterface
    UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
#pragma endregion // UActorComponentInterface

    UE_API virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;

    UE_API void CancelAbilitiesByFunc(const TShouldCancelAbilityFunc& ShouldCancelFunc, bool bReplicateCancelAbility);

    UE_API void CancelInputActivatedAbilities(bool bReplicateCancelAbility);

    UE_API void AbilityInputTagPressed(const FGameplayTag& InputTag);
    UE_API void AbilityInputTagReleased(const FGameplayTag& InputTag);

    UE_API void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
    UE_API void ClearAbilityInput();

    UE_API bool IsActivationGroupBlocked(EGameAbilityActivationGroup InGroup) const;
    UE_API void AddAbilityToActivationGroup(EGameAbilityActivationGroup InGroup, UUR_GameplayAbility* InGameAbility);
    UE_API void RemoveAbilityFromActivationGroup(EGameAbilityActivationGroup InGroup, const UUR_GameplayAbility* InAbility);
    UE_API void CancelActivationGroupAbilities(EGameAbilityActivationGroup InGroup, UUR_GameplayAbility* InIgnoreGameAbility, const bool bReplicateCancelAbility);

    // Uses a gameplay effect to add the specified dynamic granted tag.
    UE_API void AddDynamicTagGameplayEffect(const FGameplayTag& Tag);

    // Removes all active instances of the gameplay effect that was used to add the specified dynamic granted tag.
    UE_API void RemoveDynamicTagGameplayEffect(const FGameplayTag& Tag);

    /** Gets the ability target data associated with the given ability handle and activation info */
    UE_API void GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle, const FGameplayAbilityActivationInfo& ActivationInfo, FGameplayAbilityTargetDataHandle& OutTargetDataHandle);

    /** Sets the current tag relationship mapping, if null it will clear it out */
    UE_API void SetTagRelationshipMapping(UUR_AbilityTagRelationshipMapping* NewMapping);

    /** Looks at ability tags and gathers additional required and blocking tags */
    UE_API void GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer& OutActivationRequired, FGameplayTagContainer& OutActivationBlocked) const;

    UE_API void TryActivateAbilitiesOnSpawn();

    /////////////////////////////////////////////////////////////////////////////////////////////////
protected:
    UE_API virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
    UE_API virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;

    UE_API virtual void NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability) override;
    UE_API virtual void NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason) override;
    UE_API virtual void NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled) override;
    UE_API virtual void ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags) override;
    UE_API virtual void HandleChangeAbilityCanBeCanceled(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bCanBeCanceled) override;

    /** Notify client that an ability failed to activate */
    UFUNCTION(Client, Unreliable)
    UE_API void ClientNotifyAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);

    UE_API void HandleAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);

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

#undef UE_API
