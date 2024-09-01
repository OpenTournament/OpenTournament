// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_GameplayAbility.h"

#include <AbilitySystemBlueprintLibrary.h>
#include <AbilitySystemLog.h>
#include <NativeGameplayTags.h>

#include "UR_AbilitySystemComponent.h"
#include "UR_Character.h"
#include "UR_TargetType.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_GameplayAbility)

/////////////////////////////////////////////////////////////////////////////////////////////////

#define ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(FunctionName, ReturnValue)																				\
{																																						\
	if (!ensure(IsInstantiated()))																														\
	{																																					\
		ABILITY_LOG(Error, TEXT("%s: " #FunctionName " cannot be called on a non-instanced ability. Check the instancing policy."), *GetPathName());	\
		return ReturnValue;																																\
	}																																					\
}

UE_DEFINE_GAMEPLAY_TAG(TAG_ABILITY_SIMPLE_FAILURE_MESSAGE, "Ability.UserFacingSimpleActivateFail.Message");
UE_DEFINE_GAMEPLAY_TAG(TAG_ABILITY_PLAY_MONTAGE_FAILURE_MESSAGE, "Ability.PlayMontageOnActivateFail.Message");

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_GameplayAbility::UUR_GameplayAbility(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
    NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;

    ActivationPolicy = EGameAbilityActivationPolicy::OnInputTriggered;
    ActivationGroup = EGameAbilityActivationGroup::Independent;

    //bLogCancelation = false;

    //ActiveCameraMode = nullptr;
}

UUR_AbilitySystemComponent* UUR_GameplayAbility::GetGameAbilitySystemComponentFromActorInfo() const
{
    return (CurrentActorInfo ? Cast<UUR_AbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent.Get()) : nullptr);
}

bool UUR_GameplayAbility::CanChangeActivationGroup(EGameAbilityActivationGroup NewGroup) const
{
    if (!IsInstantiated() || !IsActive())
    {
        return false;
    }

    if (ActivationGroup == NewGroup)
    {
        return true;
    }

    UUR_AbilitySystemComponent* ASC = GetGameAbilitySystemComponentFromActorInfo();
    check(ASC);

    if ((ActivationGroup != EGameAbilityActivationGroup::Exclusive_Blocking) && ASC->IsActivationGroupBlocked(NewGroup))
    {
        // This ability can't change groups if it's blocked (unless it is the one doing the blocking).
        return false;
    }

    if ((NewGroup == EGameAbilityActivationGroup::Exclusive_Replaceable) && !CanBeCanceled())
    {
        // This ability can't become replaceable if it can't be canceled.
        return false;
    }

    return true;
}

bool UUR_GameplayAbility::ChangeActivationGroup(EGameAbilityActivationGroup NewGroup)
{
    ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(ChangeActivationGroup, false);

    if (!CanChangeActivationGroup(NewGroup))
    {
        return false;
    }

    if (ActivationGroup != NewGroup)
    {
        UUR_AbilitySystemComponent* ASC = GetGameAbilitySystemComponentFromActorInfo();
        check(ASC);

        ASC->RemoveAbilityFromActivationGroup(ActivationGroup, this);
        ASC->AddAbilityToActivationGroup(NewGroup, this);

        ActivationGroup = NewGroup;
    }

    return true;
}

FUR_GameplayEffectContainerSpec UUR_GameplayAbility::MakeEffectContainerSpecFromContainer(const FUR_GameplayEffectContainer& Container, const FGameplayEventData& EventData, int32 OverrideGameplayLevel)
{
    // First figure out our actor info
    FUR_GameplayEffectContainerSpec ReturnSpec;
    AActor* OwningActor = GetOwningActorFromActorInfo();
    AUR_Character* OwningCharacter = Cast<AUR_Character>(OwningActor);
    UUR_AbilitySystemComponent* OwningAbilityComponent = UUR_AbilitySystemComponent::GetAbilitySystemComponentFromActor(OwningActor);

    if (OwningAbilityComponent)
    {
        // If we have a target type, run the targeting logic. This is optional, targets can be added later
        if (Container.TargetType.Get())
        {
            TArray<FHitResult> HitResults;
            TArray<AActor*> TargetActors;
            const UUR_TargetType* TargetTypeCDO = Container.TargetType.GetDefaultObject();
            AActor* AvatarActor = GetAvatarActorFromActorInfo();
            TargetTypeCDO->GetTargets(OwningCharacter, AvatarActor, EventData, HitResults, TargetActors);
            ReturnSpec.AddTargets(HitResults, TargetActors);
        }

        // If we don't have an override level, use the default ont he ability system component
        if (OverrideGameplayLevel == INDEX_NONE)
        {
            OverrideGameplayLevel = OwningAbilityComponent->GetDefaultAbilityLevel();
        }

        // Build GameplayEffectSpecs for each applied effect
        for (const TSubclassOf<UGameplayEffect>& EffectClass : Container.TargetGameplayEffectClasses)
        {
            ReturnSpec.TargetGameplayEffectSpecs.Add(MakeOutgoingGameplayEffectSpec(EffectClass, OverrideGameplayLevel));
        }
    }
    return ReturnSpec;
}

FUR_GameplayEffectContainerSpec UUR_GameplayAbility::MakeEffectContainerSpec(FGameplayTag ContainerTag, const FGameplayEventData& EventData, int32 OverrideGameplayLevel)
{
    FUR_GameplayEffectContainer* FoundContainer = EffectContainerMap.Find(ContainerTag);

    if (FoundContainer)
    {
        return MakeEffectContainerSpecFromContainer(*FoundContainer, EventData, OverrideGameplayLevel);
    }
    return FUR_GameplayEffectContainerSpec();
}

TArray<FActiveGameplayEffectHandle> UUR_GameplayAbility::ApplyEffectContainerSpec(const FUR_GameplayEffectContainerSpec& ContainerSpec)
{
    TArray<FActiveGameplayEffectHandle> AllEffects;

    // Iterate list of effect specs and apply them to their target data
    for (const FGameplayEffectSpecHandle& SpecHandle : ContainerSpec.TargetGameplayEffectSpecs)
    {
        AllEffects.Append(K2_ApplyGameplayEffectSpecToTarget(SpecHandle, ContainerSpec.TargetData));
    }
    return AllEffects;
}

TArray<FActiveGameplayEffectHandle> UUR_GameplayAbility::ApplyEffectContainer(FGameplayTag ContainerTag, const FGameplayEventData& EventData, int32 OverrideGameplayLevel)
{
    FUR_GameplayEffectContainerSpec Spec = MakeEffectContainerSpec(ContainerTag, EventData, OverrideGameplayLevel);
    return ApplyEffectContainerSpec(Spec);
}
