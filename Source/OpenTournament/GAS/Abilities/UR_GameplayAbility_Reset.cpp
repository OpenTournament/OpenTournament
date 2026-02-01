// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "GAS/Abilities/UR_GameplayAbility_Reset.h"

#include "GameFramework/GameplayMessageSubsystem.h"

#include "UR_Character.h"
#include "UR_GameplayTags.h"
#include "GAS/UR_AbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_GameplayAbility_Reset)

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_GameplayAbility_Reset::UUR_GameplayAbility_Reset(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

    if (HasAnyFlags(RF_ClassDefaultObject))
    {
        // Add the ability trigger tag as default to the CDO.
        FAbilityTriggerData TriggerData;
        TriggerData.TriggerTag = URGameplayTags::GameplayEvent_RequestReset;
        TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
        AbilityTriggers.Add(TriggerData);
    }
}

void UUR_GameplayAbility_Reset::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    check(ActorInfo);

    UUR_AbilitySystemComponent* ASC = CastChecked<UUR_AbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());

    FGameplayTagContainer AbilityTypesToIgnore;
    AbilityTypesToIgnore.AddTag(URGameplayTags::Ability_Behavior_SurvivesDeath);

    // Cancel all abilities and block others from starting.
    ASC->CancelAbilities(nullptr, &AbilityTypesToIgnore, this);

    SetCanBeCanceled(false);

    // Execute the reset from the character
    if (AUR_Character* Character = Cast<AUR_Character>(CurrentActorInfo->AvatarActor.Get()))
    {
        Character->Reset();
    }

    // Let others know a reset has occurred
    FUR_PlayerResetMessage Message;
    Message.OwnerPlayerState = CurrentActorInfo->OwnerActor.Get();
    UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(this);
    MessageSystem.BroadcastMessage(URGameplayTags::GameplayEvent_Reset, Message);

    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    const bool bReplicateEndAbility = true;
    const bool bWasCanceled = false;
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEndAbility, bWasCanceled);
}
