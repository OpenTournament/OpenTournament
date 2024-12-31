// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_GameplayAbility.h"

#include <AbilitySystemBlueprintLibrary.h>
#include <AbilitySystemLog.h>
#include <NativeGameplayTags.h>

#include "AbilitySystemGlobals.h"
#include "UR_AbilitySourceInterface.h"
#include "UR_AbilitySystemComponent.h"
#include "UR_Character.h"
#include "UR_GameplayEffectContext.h"
#include "UR_GameplayTags.h"
#include "UR_HeroComponent.h"
#include "UR_LogChannels.h"
#include "UR_PlayerController.h"
#include "UR_TargetType.h"
#include "Abilities/UR_AbilityCost.h"
#include "Abilities/UR_AbilityMontageFailureMessage.h"
#include "Abilities/UR_AbilitySimpleFailureMessage.h"
#include "Camera/UR_CameraMode.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Physics/PhysicalMaterialWithTags.h"

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

    bLogCancellation = false;

    ActiveCameraMode = nullptr;
}

UUR_AbilitySystemComponent* UUR_GameplayAbility::GetGameAbilitySystemComponentFromActorInfo() const
{
    return (CurrentActorInfo ? Cast<UUR_AbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent.Get()) : nullptr);
}

AUR_PlayerController* UUR_GameplayAbility::GetGamePlayerControllerFromActorInfo() const
{
    return (CurrentActorInfo ? Cast<AUR_PlayerController>(CurrentActorInfo->PlayerController.Get()) : nullptr);
}

AController* UUR_GameplayAbility::GetControllerFromActorInfo() const
{
    if (CurrentActorInfo)
    {
        if (AController* PC = CurrentActorInfo->PlayerController.Get())
        {
            return PC;
        }

        // Look for a player controller or pawn in the owner chain.
        AActor* TestActor = CurrentActorInfo->OwnerActor.Get();
        while (TestActor)
        {
            if (AController* C = Cast<AController>(TestActor))
            {
                return C;
            }

            if (const APawn* Pawn = Cast<APawn>(TestActor))
            {
                return Pawn->GetController();
            }

            TestActor = TestActor->GetOwner();
        }
    }

    return nullptr;
}

AUR_Character* UUR_GameplayAbility::GetGameCharacterFromActorInfo() const
{
    return (CurrentActorInfo ? Cast<AUR_Character>(CurrentActorInfo->AvatarActor.Get()) : nullptr);
}

UUR_HeroComponent* UUR_GameplayAbility::GetHeroComponentFromActorInfo() const
{
    return (CurrentActorInfo ? UUR_HeroComponent::FindHeroComponent(CurrentActorInfo->AvatarActor.Get()) : nullptr);
}

void UUR_GameplayAbility::TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const
{
    // Try to activate if activation policy is on spawn.
    if (ActorInfo && !Spec.IsActive() && (ActivationPolicy == EGameAbilityActivationPolicy::OnSpawn))
    {
        UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
        const AActor* AvatarActor = ActorInfo->AvatarActor.Get();

        // If avatar actor is torn off or about to die, don't try to activate until we get the new one.
        if (ASC && AvatarActor && !AvatarActor->GetTearOff() && (AvatarActor->GetLifeSpan() <= 0.0f))
        {
            const bool bIsLocalExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalPredicted) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalOnly);
            const bool bIsServerExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerOnly) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerInitiated);

            const bool bClientShouldActivate = ActorInfo->IsLocallyControlled() && bIsLocalExecution;
            const bool bServerShouldActivate = ActorInfo->IsNetAuthority() && bIsServerExecution;

            if (bClientShouldActivate || bServerShouldActivate)
            {
                ASC->TryActivateAbility(Spec.Handle);
            }
        }
    }
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

void UUR_GameplayAbility::SetCameraMode(TSubclassOf<UUR_CameraMode> CameraMode)
{
    ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(SetCameraMode,);

    if (UUR_HeroComponent* HeroComponent = GetHeroComponentFromActorInfo())
    {
        HeroComponent->SetAbilityCameraMode(CameraMode, CurrentSpecHandle);
        ActiveCameraMode = CameraMode;
    }
}

void UUR_GameplayAbility::ClearCameraMode()
{
    ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(ClearCameraMode,);

    if (ActiveCameraMode)
    {
        if (UUR_HeroComponent* HeroComponent = GetHeroComponentFromActorInfo())
        {
            HeroComponent->ClearAbilityCameraMode(CurrentSpecHandle);
        }

        ActiveCameraMode = nullptr;
    }
}

void UUR_GameplayAbility::OnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const
{
    NativeOnAbilityFailedToActivate(FailedReason);
    ScriptOnAbilityFailedToActivate(FailedReason);
}

void UUR_GameplayAbility::NativeOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const
{
    bool bSimpleFailureFound = false;
    for (const FGameplayTag& Reason : FailedReason)
    {
        if (!bSimpleFailureFound)
        {
            if (const FText* UserFacingMessage = FailureTagToUserFacingMessages.Find(Reason))
            {
                FGameAbilitySimpleFailureMessage Message;
                Message.PlayerController = GetActorInfo().PlayerController.Get();
                Message.FailureTags = FailedReason;
                Message.UserFacingReason = *UserFacingMessage;

                UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
                MessageSystem.BroadcastMessage(TAG_ABILITY_SIMPLE_FAILURE_MESSAGE, Message);
                bSimpleFailureFound = true;
            }
        }

        if (UAnimMontage* Montage = FailureTagToAnimMontage.FindRef(Reason))
        {
            FGameAbilityMontageFailureMessage Message;
            Message.PlayerController = GetActorInfo().PlayerController.Get();
            Message.AvatarActor = GetActorInfo().AvatarActor.Get();
            Message.FailureTags = FailedReason;
            Message.FailureMontage = Montage;

            UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
            MessageSystem.BroadcastMessage(TAG_ABILITY_PLAY_MONTAGE_FAILURE_MESSAGE, Message);
        }
    }
}

bool UUR_GameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
    {
        return false;
    }

    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }

    //@TODO Possibly remove after setting up tag relationships
    UUR_AbilitySystemComponent* ASC = CastChecked<UUR_AbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
    if (ASC->IsActivationGroupBlocked(ActivationGroup))
    {
        if (OptionalRelevantTags)
        {
            OptionalRelevantTags->AddTag(URGameplayTags::Ability_ActivateFail_ActivationGroup);
        }
        return false;
    }

    return true;
}

void UUR_GameplayAbility::SetCanBeCanceled(bool bCanBeCanceled)
{
    // The ability can not block canceling if it's replaceable.
    if (!bCanBeCanceled && (ActivationGroup == EGameAbilityActivationGroup::Exclusive_Replaceable))
    {
        UE_LOG(LogGameAbilitySystem, Error, TEXT("SetCanBeCanceled: Ability [%s] can not block canceling because its activation group is replaceable."), *GetName());
        return;
    }

    Super::SetCanBeCanceled(bCanBeCanceled);
}

void UUR_GameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
    Super::OnGiveAbility(ActorInfo, Spec);

    K2_OnAbilityAdded();

    TryActivateAbilityOnSpawn(ActorInfo, Spec);
}

void UUR_GameplayAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
    K2_OnAbilityRemoved();

    Super::OnRemoveAbility(ActorInfo, Spec);
}

void UUR_GameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UUR_GameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    ClearCameraMode();

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UUR_GameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags) || !ActorInfo)
    {
        return false;
    }

    // Verify we can afford any additional costs
    for (const TObjectPtr<UUR_AbilityCost>& AdditionalCost : AdditionalCosts)
    {
        if (AdditionalCost != nullptr)
        {
            if (!AdditionalCost->CheckCost(this, Handle, ActorInfo, /*inout*/ OptionalRelevantTags))
            {
                return false;
            }
        }
    }

    return true;
}

void UUR_GameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
    Super::ApplyCost(Handle, ActorInfo, ActivationInfo);

    check(ActorInfo);

    // Used to determine if the ability actually hit a target (as some costs are only spent on successful attempts)
    auto DetermineIfAbilityHitTarget = [&]()
    {
        if (ActorInfo->IsNetAuthority())
        {
            if (UUR_AbilitySystemComponent* ASC = Cast<UUR_AbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get()))
            {
                FGameplayAbilityTargetDataHandle TargetData;
                ASC->GetAbilityTargetData(Handle, ActivationInfo, TargetData);
                for (int32 TargetDataIdx = 0; TargetDataIdx < TargetData.Data.Num(); ++TargetDataIdx)
                {
                    if (UAbilitySystemBlueprintLibrary::TargetDataHasHitResult(TargetData, TargetDataIdx))
                    {
                        return true;
                    }
                }
            }
        }

        return false;
    };

    // Pay any additional costs
    bool bAbilityHitTarget = false;
    bool bHasDeterminedIfAbilityHitTarget = false;
    for (const TObjectPtr<UUR_AbilityCost>& AdditionalCost : AdditionalCosts)
    {
        if (AdditionalCost != nullptr)
        {
            if (AdditionalCost->ShouldOnlyApplyCostOnHit())
            {
                if (!bHasDeterminedIfAbilityHitTarget)
                {
                    bAbilityHitTarget = DetermineIfAbilityHitTarget();
                    bHasDeterminedIfAbilityHitTarget = true;
                }

                if (!bAbilityHitTarget)
                {
                    continue;
                }
            }

            AdditionalCost->ApplyCost(this, Handle, ActorInfo, ActivationInfo);
        }
    }
}

FGameplayEffectContextHandle UUR_GameplayAbility::MakeEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const
{
    FGameplayEffectContextHandle ContextHandle = Super::MakeEffectContext(Handle, ActorInfo);

    FUR_GameplayEffectContext* EffectContext = FUR_GameplayEffectContext::ExtractEffectContext(ContextHandle);
    check(EffectContext);

    check(ActorInfo);

    AActor* EffectCauser = nullptr;
    const IUR_AbilitySourceInterface* AbilitySource = nullptr;
    float SourceLevel = 0.0f;
    GetAbilitySource(Handle, ActorInfo, /*out*/ SourceLevel, /*out*/ AbilitySource, /*out*/ EffectCauser);

    UObject* SourceObject = GetSourceObject(Handle, ActorInfo);

    AActor* Instigator = ActorInfo ? ActorInfo->OwnerActor.Get() : nullptr;

    EffectContext->SetAbilitySource(AbilitySource, SourceLevel);
    EffectContext->AddInstigator(Instigator, EffectCauser);
    EffectContext->AddSourceObject(SourceObject);

    return ContextHandle;
}

void UUR_GameplayAbility::ApplyAbilityTagsToGameplayEffectSpec(FGameplayEffectSpec& Spec, FGameplayAbilitySpec* AbilitySpec) const
{
    Super::ApplyAbilityTagsToGameplayEffectSpec(Spec, AbilitySpec);

    if (const FHitResult* HitResult = Spec.GetContext().GetHitResult())
    {
        if (const UPhysicalMaterialWithTags* PhysMatWithTags = Cast<const UPhysicalMaterialWithTags>(HitResult->PhysMaterial.Get()))
        {
            Spec.CapturedTargetTags.GetSpecTags().AppendTags(PhysMatWithTags->Tags);
        }
    }
}

bool UUR_GameplayAbility::DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	// Specialized version to handle death exclusion and AbilityTags expansion via ASC

	bool bBlocked = false;
	bool bMissing = false;

	UAbilitySystemGlobals& AbilitySystemGlobals = UAbilitySystemGlobals::Get();
	const FGameplayTag& BlockedTag = AbilitySystemGlobals.ActivateFailTagsBlockedTag;
	const FGameplayTag& MissingTag = AbilitySystemGlobals.ActivateFailTagsMissingTag;

	// Check if any of this ability's tags are currently blocked
	if (AbilitySystemComponent.AreAbilityTagsBlocked(GetAssetTags()))
	{
		bBlocked = true;
	}

	const UUR_AbilitySystemComponent* ASC = Cast<UUR_AbilitySystemComponent>(&AbilitySystemComponent);
	static FGameplayTagContainer AllRequiredTags;
	static FGameplayTagContainer AllBlockedTags;

	AllRequiredTags = ActivationRequiredTags;
	AllBlockedTags = ActivationBlockedTags;

	// Expand our ability tags to add additional required/blocked tags
	if (ASC)
	{
		ASC->GetAdditionalActivationTagRequirements(GetAssetTags(), AllRequiredTags, AllBlockedTags);
	}

	// Check to see the required/blocked tags for this ability
	if (AllBlockedTags.Num() || AllRequiredTags.Num())
	{
		static FGameplayTagContainer AbilitySystemComponentTags;

		AbilitySystemComponentTags.Reset();
		AbilitySystemComponent.GetOwnedGameplayTags(AbilitySystemComponentTags);

		if (AbilitySystemComponentTags.HasAny(AllBlockedTags))
		{
			if (OptionalRelevantTags && AbilitySystemComponentTags.HasTag(URGameplayTags::Status_Death))
			{
				// If player is dead and was rejected due to blocking tags, give that feedback
				OptionalRelevantTags->AddTag(URGameplayTags::Ability_ActivateFail_IsDead);
			}

			bBlocked = true;
		}

		if (!AbilitySystemComponentTags.HasAll(AllRequiredTags))
		{
			bMissing = true;
		}
	}

	if (SourceTags != nullptr)
	{
		if (SourceBlockedTags.Num() || SourceRequiredTags.Num())
		{
			if (SourceTags->HasAny(SourceBlockedTags))
			{
				bBlocked = true;
			}

			if (!SourceTags->HasAll(SourceRequiredTags))
			{
				bMissing = true;
			}
		}
	}

	if (TargetTags != nullptr)
	{
		if (TargetBlockedTags.Num() || TargetRequiredTags.Num())
		{
			if (TargetTags->HasAny(TargetBlockedTags))
			{
				bBlocked = true;
			}

			if (!TargetTags->HasAll(TargetRequiredTags))
			{
				bMissing = true;
			}
		}
	}

	if (bBlocked)
	{
		if (OptionalRelevantTags && BlockedTag.IsValid())
		{
			OptionalRelevantTags->AddTag(BlockedTag);
		}
		return false;
	}
	if (bMissing)
	{
		if (OptionalRelevantTags && MissingTag.IsValid())
		{
			OptionalRelevantTags->AddTag(MissingTag);
		}
		return false;
	}

	return true;
}

void UUR_GameplayAbility::OnPawnAvatarSet()
{
    K2_OnPawnAvatarSet();
}

void UUR_GameplayAbility::GetAbilitySource(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, float& OutSourceLevel, const IUR_AbilitySourceInterface*& OutAbilitySource, AActor*& OutEffectCauser) const
{
    OutSourceLevel = 0.0f;
    OutAbilitySource = nullptr;
    OutEffectCauser = nullptr;

    OutEffectCauser = ActorInfo->AvatarActor.Get();

    // If we were added by something that's an ability info source, use it
    UObject* SourceObject = GetSourceObject(Handle, ActorInfo);

    OutAbilitySource = Cast<IUR_AbilitySourceInterface>(SourceObject);
}


//

FUR_GameplayEffectContainerSpec UUR_GameplayAbility::MakeEffectContainerSpecFromContainer(const FUR_GameplayEffectContainer& Container, const FGameplayEventData& EventData, int32 OverrideGameplayLevel)
{
    // First figure out our actor info
    FUR_GameplayEffectContainerSpec ReturnSpec;
    // AActor* OwningActor = GetOwningActorFromActorInfo();
    // AUR_Character* OwningCharacter = Cast<AUR_Character>(OwningActor);
    // UUR_AbilitySystemComponent* OwningAbilityComponent = UUR_AbilitySystemComponent::GetAbilitySystemComponentFromActor(OwningActor);
    //
    // if (OwningAbilityComponent)
    // {
    //     // If we have a target type, run the targeting logic. This is optional, targets can be added later
    //     if (Container.TargetType.Get())
    //     {
    //         TArray<FHitResult> HitResults;
    //         TArray<AActor*> TargetActors;
    //         const UUR_TargetType* TargetTypeCDO = Container.TargetType.GetDefaultObject();
    //         AActor* AvatarActor = GetAvatarActorFromActorInfo();
    //         TargetTypeCDO->GetTargets(OwningCharacter, AvatarActor, EventData, HitResults, TargetActors);
    //         ReturnSpec.AddTargets(HitResults, TargetActors);
    //     }
    //
    //     // If we don't have an override level, use the default ont he ability system component
    //     if (OverrideGameplayLevel == INDEX_NONE)
    //     {
    //         OverrideGameplayLevel = OwningAbilityComponent->GetDefaultAbilityLevel();
    //     }
    //
    //     // Build GameplayEffectSpecs for each applied effect
    //     for (const TSubclassOf<UGameplayEffect>& EffectClass : Container.TargetGameplayEffectClasses)
    //     {
    //         ReturnSpec.TargetGameplayEffectSpecs.Add(MakeOutgoingGameplayEffectSpec(EffectClass, OverrideGameplayLevel));
    //     }
    // }
    return ReturnSpec;
}

FUR_GameplayEffectContainerSpec UUR_GameplayAbility::MakeEffectContainerSpec(FGameplayTag ContainerTag, const FGameplayEventData& EventData, int32 OverrideGameplayLevel)
{
    const FUR_GameplayEffectContainer* FoundContainer = EffectContainerMap.Find(ContainerTag);

    // if (FoundContainer)
    // {
    //     return MakeEffectContainerSpecFromContainer(*FoundContainer, EventData, OverrideGameplayLevel);
    // }
    return FUR_GameplayEffectContainerSpec();
}

TArray<FActiveGameplayEffectHandle> UUR_GameplayAbility::ApplyEffectContainerSpec(const FUR_GameplayEffectContainerSpec& ContainerSpec)
{
    TArray<FActiveGameplayEffectHandle> AllEffects;

    // // Iterate list of effect specs and apply them to their target data
    // for (const FGameplayEffectSpecHandle& SpecHandle : ContainerSpec.TargetGameplayEffectSpecs)
    // {
    //     AllEffects.Append(K2_ApplyGameplayEffectSpecToTarget(SpecHandle, ContainerSpec.TargetData));
    // }
    return AllEffects;
}

TArray<FActiveGameplayEffectHandle> UUR_GameplayAbility::ApplyEffectContainer(FGameplayTag ContainerTag, const FGameplayEventData& EventData, int32 OverrideGameplayLevel)
{
    FUR_GameplayEffectContainerSpec Spec = {}; //MakeEffectContainerSpec(ContainerTag, EventData, OverrideGameplayLevel);
    return ApplyEffectContainerSpec(Spec);
}
