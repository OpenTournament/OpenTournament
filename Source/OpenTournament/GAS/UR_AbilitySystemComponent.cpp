// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_AbilitySystemComponent.h"

#include <AbilitySystemGlobals.h>

#include "UR_AbilityTagRelationshipMapping.h"
#include "UR_AssetManager.h"
#include "UR_AttributeSet.h"
#include "UR_Character.h"
#include "UR_GameData.h"
#include "UR_GameplayAbility.h"
#include "UR_GlobalAbilitySystem.h"
#include "UR_LogChannels.h"
#include "Animation/UR_AnimInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_AbilitySystemComponent)

#pragma optimize("", off)

/////////////////////////////////////////////////////////////////////////////////////////////////

UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_AbilityInputBlocked, "Gameplay.AbilityInputBlocked");

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_AbilitySystemComponent::UUR_AbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    InputPressedSpecHandles.Reset();
    InputReleasedSpecHandles.Reset();
    InputHeldSpecHandles.Reset();

    FMemory::Memset(ActivationGroupCounts, 0, sizeof(ActivationGroupCounts));
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

void UUR_AbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
    const FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
    check(ActorInfo);
    check(InOwnerActor);

    const bool bHasNewPawnAvatar = Cast<APawn>(InAvatarActor) && (InAvatarActor != ActorInfo->AvatarActor);

    Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

    if (bHasNewPawnAvatar)
    {
        // Notify all abilities that a new pawn avatar has been set
        for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
        {
            ensureMsgf(AbilitySpec.Ability && AbilitySpec.Ability->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::InstancedPerActor, TEXT("InitAbilityActorInfo: All Abilities should be Instanced (NonInstanced is being deprecated due to usability issues)."));

            TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
            for (UGameplayAbility* AbilityInstance : Instances)
            {
                UUR_GameplayAbility* GameAbilityInstance = Cast<UUR_GameplayAbility>(AbilityInstance);
                if (GameAbilityInstance)
                {
                    // Ability instances may be missing for replays
                    GameAbilityInstance->OnPawnAvatarSet();
                }
            }
        }

        // Register with the global system once we actually have a pawn avatar. We wait until this time since some globally-applied effects may require an avatar.
        if (UUR_GlobalAbilitySystem* GlobalAbilitySystem = UWorld::GetSubsystem<UUR_GlobalAbilitySystem>(GetWorld()))
        {
            GlobalAbilitySystem->RegisterASC(this);
        }

        if (UUR_AnimInstance* AnimInst = Cast<UUR_AnimInstance>(ActorInfo->GetAnimInstance()))
        {
            AnimInst->InitializeWithAbilitySystem(this);
        }

        TryActivateAbilitiesOnSpawn();
    }
}

void UUR_AbilitySystemComponent::CancelAbilitiesByFunc(const TShouldCancelAbilityFunc& ShouldCancelFunc, bool bReplicateCancelAbility)
{
    ABILITYLIST_SCOPE_LOCK();
    for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
    {
        if (!AbilitySpec.IsActive())
        {
            continue;
        }

        const auto* AbilityCDO = Cast<UUR_GameplayAbility>(AbilitySpec.Ability);
        if (!AbilityCDO)
        {
            UE_LOG(LogGameAbilitySystem, Error, TEXT("CancelAbilitiesByFunc: Non-URGameplayAbility %s was Granted to ASC. Skipping."), *AbilitySpec.Ability.GetName());
            continue;
        }

        ensureMsgf(AbilitySpec.Ability->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::InstancedPerActor, TEXT("CancelAbilitiesByFunc: All Abilities should be Instanced (NonInstanced is being deprecated due to usability issues)."));

        // Cancel all the spawned instances.
        TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
        for (auto* AbilityInstance : Instances)
        {
            UUR_GameplayAbility* GameAbilityInstance = CastChecked<UUR_GameplayAbility>(AbilityInstance);

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
}

void UUR_AbilitySystemComponent::CancelInputActivatedAbilities(bool bReplicateCancelAbility)
{
    auto ShouldCancelFunc = [this](const UUR_GameplayAbility* GameAbility, FGameplayAbilitySpecHandle Handle)
    {
        const EGameAbilityActivationPolicy ActivationPolicy = GameAbility->GetActivationPolicy();
        return ((ActivationPolicy == EGameAbilityActivationPolicy::OnInputTriggered) || (ActivationPolicy == EGameAbilityActivationPolicy::WhileInputActive));
    };

    CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}

void UUR_AbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
    if (InputTag.IsValid())
    {
        for (FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
        {
            if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
            {
                // Store the InputTag in the EventData
                TSharedPtr<FGameplayEventData> EventData = MakeShareable(new FGameplayEventData());
                EventData->EventTag = InputTag;
                EventData->Instigator = GetAvatarActor();
                EventData->Target = GetAvatarActor();
                AbilitySpec.GameplayEventData = EventData;

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
            if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
            {
                InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
                InputHeldSpecHandles.Remove(AbilitySpec.Handle);
            }
        }
    }
}

void UUR_AbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
    if (HasMatchingGameplayTag(TAG_Gameplay_AbilityInputBlocked))
    {
        ClearAbilityInput();
        return;
    }

    static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
    AbilitiesToActivate.Reset();

    //@TODO: See if we can use FScopedServerAbilityRPCBatcher ScopedRPCBatcher in some of these loops

    // Process all abilities that activate when the input is held.
    for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
    {
        if (const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
        {
            if (AbilitySpec->Ability && !AbilitySpec->IsActive())
            {
                const UUR_GameplayAbility* AbilityCDO = Cast<UUR_GameplayAbility>(AbilitySpec->Ability);
                if (AbilityCDO && AbilityCDO->GetActivationPolicy() == EGameAbilityActivationPolicy::WhileInputActive)
                {
                    AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
                }
            }
        }
    }

    // Process all abilities that had their input pressed this frame.
    for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
    {
        if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
        {
            if (AbilitySpec->Ability)
            {
                AbilitySpec->InputPressed = true;

                if (AbilitySpec->IsActive())
                {
                    // Ability is active so pass along the input event.
                    AbilitySpecInputPressed(*AbilitySpec);
                }
                else
                {
                    const UUR_GameplayAbility* AbilityCDO = Cast<UUR_GameplayAbility>(AbilitySpec->Ability);

                    if (AbilityCDO && AbilityCDO->GetActivationPolicy() == EGameAbilityActivationPolicy::OnInputTriggered)
                    {
                        AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
                    }
                }
            }
        }
    }

    // Try to activate all the abilities that are from presses and holds.
    // We do it all at once so that held inputs don't activate the ability
    // and then also send a input event to the ability because of the press.
    for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate)
    {
        TryActivateAbility(AbilitySpecHandle);
    }

    // Process all abilities that had their input released this frame.
    for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
    {
        if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
        {
            if (AbilitySpec->Ability)
            {
                AbilitySpec->InputPressed = false;

                if (AbilitySpec->IsActive())
                {
                    // Ability is active so pass along the input event.
                    AbilitySpecInputReleased(*AbilitySpec);
                }
            }
        }
    }

    // Clear the cached ability handles.
    InputPressedSpecHandles.Reset();
    InputReleasedSpecHandles.Reset();
}

void UUR_AbilitySystemComponent::ClearAbilityInput()
{
    InputPressedSpecHandles.Reset();
    InputReleasedSpecHandles.Reset();
    InputHeldSpecHandles.Reset();
}

bool UUR_AbilitySystemComponent::IsActivationGroupBlocked(EGameAbilityActivationGroup InGroup) const
{
    bool bBlocked = false;

    switch (InGroup)
    {
        case EGameAbilityActivationGroup::Independent:
        {
            // Independent abilities are never blocked.
            bBlocked = false;
            break;
        }
        case EGameAbilityActivationGroup::Exclusive_Replaceable:
        case EGameAbilityActivationGroup::Exclusive_Blocking:
        {
            // Exclusive abilities can activate if nothing is blocking.
            bBlocked = (ActivationGroupCounts[static_cast<uint8>(EGameAbilityActivationGroup::Exclusive_Blocking)] > 0);
            break;
        }
        default:
        {
            checkf(false, TEXT("IsActivationGroupBlocked: Invalid ActivationGroup [%d]\n"), static_cast<uint8>(InGroup));
            break;
        }
    }

    return bBlocked;
}

void UUR_AbilitySystemComponent::AddAbilityToActivationGroup(EGameAbilityActivationGroup InGroup, UUR_GameplayAbility* InGameAbility)
{
    check(InGameAbility);
    check(ActivationGroupCounts[static_cast<uint8>(InGroup)] < INT32_MAX);

    ActivationGroupCounts[static_cast<uint8>(InGroup)]++;

    constexpr bool bReplicateCancelAbility = false;

    switch (InGroup)
    {
        case EGameAbilityActivationGroup::Independent:
        {
            // Independent abilities do not cancel any other abilities.
            break;
        }
        case EGameAbilityActivationGroup::Exclusive_Replaceable:
        case EGameAbilityActivationGroup::Exclusive_Blocking:
        {
            CancelActivationGroupAbilities(EGameAbilityActivationGroup::Exclusive_Replaceable, InGameAbility, bReplicateCancelAbility);
            break;
        }
        default:
        {
            checkf(false, TEXT("AddAbilityToActivationGroup: Invalid ActivationGroup [%d]\n"), static_cast<uint8>(InGroup));
            break;
        }
    }

    const int32 ExclusiveCount = ActivationGroupCounts[static_cast<uint8>(EGameAbilityActivationGroup::Exclusive_Replaceable)] + ActivationGroupCounts[static_cast<uint8>(EGameAbilityActivationGroup::Exclusive_Blocking)];
    if (!ensure(ExclusiveCount <= 1))
    {
        UE_LOG(LogGameAbilitySystem, Error, TEXT("AddAbilityToActivationGroup: Multiple exclusive abilities are running."));
    }
}

void UUR_AbilitySystemComponent::RemoveAbilityFromActivationGroup(EGameAbilityActivationGroup InGroup, const UUR_GameplayAbility* InAbility)
{
    check(InAbility);
    check(ActivationGroupCounts[static_cast<uint8>(InGroup)] > 0);

    ActivationGroupCounts[static_cast<uint8>(InGroup)]--;
}

void UUR_AbilitySystemComponent::CancelActivationGroupAbilities(EGameAbilityActivationGroup InGroup, UUR_GameplayAbility* InIgnoreGameAbility, const bool bReplicateCancelAbility)
{
    auto ShouldCancelFunc = [this, InGroup, InIgnoreGameAbility](const UUR_GameplayAbility* InGameAbility, FGameplayAbilitySpecHandle Handle)
    {
        return ((InGameAbility->GetActivationGroup() == InGroup) && (InGameAbility != InIgnoreGameAbility));
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

void UUR_AbilitySystemComponent::GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle, const FGameplayAbilityActivationInfo& ActivationInfo, FGameplayAbilityTargetDataHandle& OutTargetDataHandle)
{
    const TSharedPtr<FAbilityReplicatedDataCache> ReplicatedData = AbilityTargetDataMap.Find(FGameplayAbilitySpecHandleAndPredictionKey(AbilityHandle, ActivationInfo.GetActivationPredictionKey()));
    if (ReplicatedData.IsValid())
    {
        OutTargetDataHandle = ReplicatedData->TargetData;
    }
}

void UUR_AbilitySystemComponent::SetTagRelationshipMapping(UUR_AbilityTagRelationshipMapping* NewMapping)
{
    TagRelationshipMapping = NewMapping;
}

void UUR_AbilitySystemComponent::GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer& OutActivationRequired, FGameplayTagContainer& OutActivationBlocked) const
{
    if (TagRelationshipMapping)
    {
        TagRelationshipMapping->GetRequiredAndBlockedActivationTags(AbilityTags, &OutActivationRequired, &OutActivationBlocked);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_AbilitySystemComponent::TryActivateAbilitiesOnSpawn()
{
    ABILITYLIST_SCOPE_LOCK();
    for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
    {
        if (const UUR_GameplayAbility* GameAbilityCDO = Cast<UUR_GameplayAbility>(AbilitySpec.Ability))
        {
            GameAbilityCDO->TryActivateAbilityOnSpawn(AbilityActorInfo.Get(), AbilitySpec);
        }
    }
}

void UUR_AbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
    Super::AbilitySpecInputPressed(Spec);

    // We don't support UGameplayAbility::bReplicateInputDirectly.
    // Use replicated events instead so that the WaitInputPress ability task works.
    if (Spec.IsActive())
    {
        const UGameplayAbility* Instance = Spec.GetPrimaryInstance();
        FPredictionKey OriginalPredictionKey = Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey() : Instance->GetCurrentActivationInfo().GetActivationPredictionKey();

        // Invoke the InputPressed event. This is not replicated here. If someone is listening, they may replicate the InputPressed event to the server.
        InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, OriginalPredictionKey);
    }
}

void UUR_AbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
    Super::AbilitySpecInputReleased(Spec);

    // We don't support UGameplayAbility::bReplicateInputDirectly.
    // Use replicated events instead so that the WaitInputRelease ability task works.
    if (Spec.IsActive())
    {
        const UGameplayAbility* Instance = Spec.GetPrimaryInstance();
        const FPredictionKey OriginalPredictionKey = Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey() : Instance->GetCurrentActivationInfo().GetActivationPredictionKey();

        // Invoke the InputReleased event. This is not replicated here. If someone is listening, they may replicate the InputReleased event to the server.
        InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, OriginalPredictionKey);
    }
}

void UUR_AbilitySystemComponent::NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability)
{
    Super::NotifyAbilityActivated(Handle, Ability);

    if (UUR_GameplayAbility* GameplayAbility = Cast<UUR_GameplayAbility>(Ability))
    {
        AddAbilityToActivationGroup(GameplayAbility->GetActivationGroup(), GameplayAbility);
    }
}

void UUR_AbilitySystemComponent::NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
    Super::NotifyAbilityFailed(Handle, Ability, FailureReason);

    if (const auto* Avatar = Cast<APawn>(GetAvatarActor()))
    {
        if (!Avatar->IsLocallyControlled() && Ability->IsSupportedForNetworking())
        {
            ClientNotifyAbilityFailed(Ability, FailureReason);
            return;
        }
    }

    HandleAbilityFailed(Ability, FailureReason);
}

void UUR_AbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled)
{
    Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);

    if (UUR_GameplayAbility* GameplayAbility = Cast<UUR_GameplayAbility>(Ability))
    {
        RemoveAbilityFromActivationGroup(GameplayAbility->GetActivationGroup(), GameplayAbility);
    }
}

void UUR_AbilitySystemComponent::ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags)
{
    FGameplayTagContainer ModifiedBlockTags = BlockTags;
    FGameplayTagContainer ModifiedCancelTags = CancelTags;

    if (TagRelationshipMapping)
    {
        // Use the mapping to expand the ability tags into block and cancel tag
        TagRelationshipMapping->GetAbilityTagsToBlockAndCancel(AbilityTags, &ModifiedBlockTags, &ModifiedCancelTags);
    }

    Super::ApplyAbilityBlockAndCancelTags(AbilityTags, RequestingAbility, bEnableBlockTags, ModifiedBlockTags, bExecuteCancelTags, ModifiedCancelTags);

    //@TODO: Apply any special logic like blocking input or movement
}

void UUR_AbilitySystemComponent::HandleChangeAbilityCanBeCanceled(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bCanBeCanceled)
{
    Super::HandleChangeAbilityCanBeCanceled(AbilityTags, RequestingAbility, bCanBeCanceled);

    //@TODO: Apply any special logic like blocking input or movement
}

void UUR_AbilitySystemComponent::ClientNotifyAbilityFailed_Implementation(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
    HandleAbilityFailed(Ability, FailureReason);
}

void UUR_AbilitySystemComponent::HandleAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
    //UE_LOG(LogGameAbilitySystem, Warning, TEXT("Ability %s failed to activate (tags: %s)"), *GetPathNameSafe(Ability), *FailureReason.ToString());

    if (const UUR_GameplayAbility* GameplayAbility = Cast<const UUR_GameplayAbility>(Ability))
    {
        GameplayAbility->OnAbilityFailedToActivate(FailureReason);
    }
}

#pragma optimize("", on)

/////////////////////////////////////////////////////////////////////////////////////////////////
