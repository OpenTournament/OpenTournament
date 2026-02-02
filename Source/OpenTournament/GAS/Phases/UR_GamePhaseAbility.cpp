// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_GamePhaseAbility.h"

#include "AbilitySystemComponent.h"
#include "Engine/World.h"

#include "UR_GamePhaseSubsystem.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_GamePhaseAbility)

/////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "UUR_GamePhaseAbility"

UUR_GamePhaseAbility::UUR_GamePhaseAbility(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
    NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnly;
}

void UUR_GamePhaseAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (ActorInfo->IsNetAuthority())
    {
        UWorld* World = ActorInfo->AbilitySystemComponent->GetWorld();
        UUR_GamePhaseSubsystem* PhaseSubsystem = UWorld::GetSubsystem<UUR_GamePhaseSubsystem>(World);
        PhaseSubsystem->OnBeginPhase(this, Handle);
    }

    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UUR_GamePhaseAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    if (ActorInfo->IsNetAuthority())
    {
        UWorld* World = ActorInfo->AbilitySystemComponent->GetWorld();
        UUR_GamePhaseSubsystem* PhaseSubsystem = UWorld::GetSubsystem<UUR_GamePhaseSubsystem>(World);
        PhaseSubsystem->OnEndPhase(this, Handle);
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

#if WITH_EDITOR
EDataValidationResult UUR_GamePhaseAbility::IsDataValid(FDataValidationContext& Context) const
{
    EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

    if (!GamePhaseTag.IsValid())
    {
        Result = EDataValidationResult::Invalid;
        Context.AddError(LOCTEXT("GamePhaseTagNotSet", "GamePhaseTag must be set to a tag representing the current phase."));
    }

    return Result;
}
#endif

#undef LOCTEXT_NAMESPACE
