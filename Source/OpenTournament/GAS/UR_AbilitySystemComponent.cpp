// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_AbilitySystemComponent.h"

#include <Engine/World.h>

#include "AbilitySystemGlobals.h"
#include "UR_AssetManager.h"

#include "UR_AttributeSet.h"
#include "UR_Character.h"
#include "UR_GameData.h"
#include "UR_GameplayAbility.h"
#include "UR_GlobalAbilitySystem.h"
#include "UR_LogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_AbilitySystemComponent)

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_AbilitySystemComponent::UUR_AbilitySystemComponent()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_AbilitySystemComponent::GetActiveAbilitiesWithTags(const FGameplayTagContainer& GameplayTagContainer, TArray<UGameplayAbility*>& ActiveAbilities) const
{
    TArray<FGameplayAbilitySpec*> AbilitiesToActivate;
    GetActivatableGameplayAbilitySpecsByAllMatchingTags(GameplayTagContainer, AbilitiesToActivate, false);

    // Iterate the list of all ability specs
    for (FGameplayAbilitySpec* Spec : AbilitiesToActivate)
    {
        // Iterate all instances on this ability spec
        TArray<UGameplayAbility*> AbilityInstances = Spec->GetAbilityInstances();

        for (UGameplayAbility* ActiveAbility : AbilityInstances)
        {
            ActiveAbilities.Add(Cast<UGameplayAbility>(ActiveAbility));
        }
    }
}

int32 UUR_AbilitySystemComponent::GetDefaultAbilityLevel() const
{
    AUR_Character* OwningCharacter = Cast<AUR_Character>(GetOwnerActor());

    if (OwningCharacter)
    {
        //return OwningCharacter->GetCharacterLevel();
    }
    return 1;
}

UUR_AbilitySystemComponent* UUR_AbilitySystemComponent::GetAbilitySystemComponentFromActor(const AActor* Actor, bool LookForComponent)
{
    return Cast<UUR_AbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor, LookForComponent));
}

void UUR_AbilitySystemComponent::ClearAbilityInput()
{
    InputPressedSpecHandles.Reset();
    InputReleasedSpecHandles.Reset();
    InputHeldSpecHandles.Reset();
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_AbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UUR_GlobalAbilitySystem* GlobalAbilitySystem = UWorld::GetSubsystem<UUR_GlobalAbilitySystem>(GetWorld()))
    {
        GlobalAbilitySystem->UnregisterASC(this);
    }

    Super::EndPlay(EndPlayReason);
}


void UUR_AbilitySystemComponent::CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc, bool bReplicateCancelAbility)
{
    ABILITYLIST_SCOPE_LOCK();
    for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
    {
        if (!AbilitySpec.IsActive())
        {
            continue;
        }

        UUR_GameplayAbility* AbilityCDO = Cast<UUR_GameplayAbility>(AbilitySpec.Ability);
        if (!AbilityCDO)
        {
            UE_LOG(LogGameAbilitySystem, Error, TEXT("CancelAbilitiesByFunc: Non-UR_GameplayAbility %s was Granted to ASC. Skipping."), *AbilitySpec.Ability.GetName());
            continue;
        }

        if (AbilityCDO->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced)
        {
            // Cancel all the spawned instances, not the CDO.
            TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
            for (UGameplayAbility* IterAbilityInstance : Instances)
            {
                UUR_GameplayAbility* GameAbilityInstance = CastChecked<UUR_GameplayAbility>(IterAbilityInstance);

                if (ShouldCancelFunc(GameAbilityInstance, AbilitySpec.Handle))
                {
                    if (GameAbilityInstance->CanBeCanceled())
                    {
                        GameAbilityInstance->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), GameAbilityInstance->GetCurrentActivationInfo(), bReplicateCancelAbility);
                    }
                    else
                    {
                        UE_LOG(LogGameAbilitySystem, Error, TEXT("CancelAbilitiesByFunc: Can't cancel ability [%s] because CanBeCanceled is false."), *GameAbilityInstance->GetName());
                    }
                }
            }
        }
        else
        {
            // Cancel the non-instanced ability CDO.
            if (ShouldCancelFunc(AbilityCDO, AbilitySpec.Handle))
            {
                // Non-instanced abilities can always be canceled.
                check(AbilityCDO->CanBeCanceled());
                AbilityCDO->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), FGameplayAbilityActivationInfo(), bReplicateCancelAbility);
            }
        }
    }
}

void UUR_AbilitySystemComponent::CancelInputActivatedAbilities(bool bReplicateCancelAbility)
{
    auto ShouldCancelFunc = [this](const UUR_GameplayAbility* LyraAbility, FGameplayAbilitySpecHandle Handle)
    {
        const EGameAbilityActivationPolicy ActivationPolicy = LyraAbility->GetActivationPolicy();
        return ((ActivationPolicy == EGameAbilityActivationPolicy::OnInputTriggered) || (ActivationPolicy == EGameAbilityActivationPolicy::WhileInputActive));
    };

    CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}

void UUR_AbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
    if (InputTag.IsValid())
    {
        for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
        {
            if (AbilitySpec.Ability && (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag)))
            {
                InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
                InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
            }
        }
    }
}

void UUR_AbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
    if (InputTag.IsValid())
    {
        for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
        {
            if (AbilitySpec.Ability && (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag)))
            {
                InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
                InputHeldSpecHandles.Remove(AbilitySpec.Handle);
            }
        }
    }
}

bool UUR_AbilitySystemComponent::IsActivationGroupBlocked(EGameAbilityActivationGroup InGroup) const
{
    bool bBlocked = false;

    switch (InGroup)
    {
        case EGameAbilityActivationGroup::Independent:
            // Independent abilities are never blocked.
                bBlocked = false;
        break;

        case EGameAbilityActivationGroup::Exclusive_Replaceable:
        case EGameAbilityActivationGroup::Exclusive_Blocking:
            // Exclusive abilities can activate if nothing is blocking.
            bBlocked = (ActivationGroupCounts[(uint8)EGameAbilityActivationGroup::Exclusive_Blocking] > 0);
        break;

        default:
            checkf(false, TEXT("IsActivationGroupBlocked: Invalid ActivationGroup [%d]\n"), (uint8)InGroup);
        break;
    }

    return bBlocked;
}

void UUR_AbilitySystemComponent::AddAbilityToActivationGroup(EGameAbilityActivationGroup InGroup, UUR_GameplayAbility* InGameAbility)
{
    check(InGameAbility);
    check(ActivationGroupCounts[(uint8)InGroup] < INT32_MAX);

    ActivationGroupCounts[(uint8)InGroup]++;

    const bool bReplicateCancelAbility = false;

    switch (InGroup)
    {
        case EGameAbilityActivationGroup::Independent:
            // Independent abilities do not cancel any other abilities.
                break;

        case EGameAbilityActivationGroup::Exclusive_Replaceable:
        case EGameAbilityActivationGroup::Exclusive_Blocking:
            CancelActivationGroupAbilities(EGameAbilityActivationGroup::Exclusive_Replaceable, InGameAbility, bReplicateCancelAbility);
        break;

        default:
            checkf(false, TEXT("AddAbilityToActivationGroup: Invalid ActivationGroup [%d]\n"), (uint8)InGroup);
        break;
    }

    const int32 ExclusiveCount = ActivationGroupCounts[(uint8)EGameAbilityActivationGroup::Exclusive_Replaceable] + ActivationGroupCounts[(uint8)EGameAbilityActivationGroup::Exclusive_Blocking];
    if (!ensure(ExclusiveCount <= 1))
    {
        UE_LOG(LogGameAbilitySystem, Error, TEXT("AddAbilityToActivationGroup: Multiple exclusive abilities are running."));
    }
}

void UUR_AbilitySystemComponent::RemoveAbilityFromActivationGroup(EGameAbilityActivationGroup InGroup, UUR_GameplayAbility* InAbility)
{
    check(InAbility);
    check(ActivationGroupCounts[(uint8)InGroup] > 0);

    ActivationGroupCounts[(uint8)InGroup]--;

}

void UUR_AbilitySystemComponent::CancelActivationGroupAbilities(EGameAbilityActivationGroup InGroup, UUR_GameplayAbility* InIgnoreLyraAbility, bool bReplicateCancelAbility)
{
    auto ShouldCancelFunc = [this, InGroup, InIgnoreLyraAbility](const UUR_GameplayAbility* InLyraAbility, FGameplayAbilitySpecHandle Handle)
    {
        return ((InLyraAbility->GetActivationGroup() == InGroup) && (InLyraAbility != InIgnoreLyraAbility));
    };

    CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}

void UUR_AbilitySystemComponent::AddDynamicTagGameplayEffect(const FGameplayTag& Tag)
{
    const auto GameplayEffectClass = UUR_GameData::Get().DynamicTagGameplayEffect;
    const TSubclassOf<UGameplayEffect> DynamicTagGE = UUR_AssetManager::GetSubclass(GameplayEffectClass);
    if (!DynamicTagGE)
    {
        UE_LOG(LogGameAbilitySystem, Warning, TEXT("AddDynamicTagGameplayEffect: Unable to find DynamicTagGameplayEffect [%s]."), *GameplayEffectClass.GetAssetName());
        return;
    }

    const FGameplayEffectSpecHandle SpecHandle = MakeOutgoingSpec(DynamicTagGE, 1.0f, MakeEffectContext());
    FGameplayEffectSpec* Spec = SpecHandle.Data.Get();

    if (!Spec)
    {
        UE_LOG(LogGameAbilitySystem, Warning, TEXT("AddDynamicTagGameplayEffect: Unable to make outgoing spec for [%s]."), *GetNameSafe(DynamicTagGE));
        return;
    }

    Spec->DynamicGrantedTags.AddTag(Tag);

    ApplyGameplayEffectSpecToSelf(*Spec);
}


void UUR_AbilitySystemComponent::RemoveDynamicTagGameplayEffect(const FGameplayTag& Tag)
{
    const auto GameplayEffectClass = UUR_GameData::Get().DynamicTagGameplayEffect;
    const TSubclassOf<UGameplayEffect> DynamicTagGE = UUR_AssetManager::GetSubclass(GameplayEffectClass);
    if (!DynamicTagGE)
    {
        UE_LOG(LogGameAbilitySystem, Warning, TEXT("RemoveDynamicTagGameplayEffect: Unable to find gameplay effect [%s]."), *GameplayEffectClass.GetAssetName());
        return;
    }

    FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(Tag));
    Query.EffectDefinition = DynamicTagGE;

    RemoveActiveEffects(Query);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_AbilitySystemComponent::SetTagRelationshipMapping(UUR_AbilityTagRelationshipMapping* NewMapping)
{
    TagRelationshipMapping = NewMapping;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

const UUR_AttributeSet* UUR_AbilitySystemComponent::GetURAttributeSetFromActor(const AActor* Actor, bool LookForComponent)
{
    if (const auto ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor, LookForComponent))
    {
        return ASC->GetSet<UUR_AttributeSet>();
    }
    return nullptr;
}
