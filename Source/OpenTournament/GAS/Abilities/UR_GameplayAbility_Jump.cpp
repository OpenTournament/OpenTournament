// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_GameplayAbility_Jump.h"

#include "UR_Character.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_GameplayAbility_Jump)

/////////////////////////////////////////////////////////////////////////////////////////////////

//struct FGameplayTagContainer;

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_GameplayAbility_Jump::UUR_GameplayAbility_Jump(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool UUR_GameplayAbility_Jump::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
    {
        return false;
    }

    const AUR_Character* Character = Cast<AUR_Character>(ActorInfo->AvatarActor.Get());
    if (!Character || !Character->CanJump())
    {
        return false;
    }

    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }

    return true;
}

void UUR_GameplayAbility_Jump::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    // Stop jumping in case the ability blueprint doesn't call it.
    CharacterJumpStop();

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UUR_GameplayAbility_Jump::CharacterJumpStart()
{
    if (AUR_Character* Character = GetGameCharacterFromActorInfo())
    {
        if (Character->IsLocallyControlled() && !Character->bPressedJump)
        {
            Character->UnCrouch();
            Character->Jump();
        }
    }
}

void UUR_GameplayAbility_Jump::CharacterJumpStop()
{
    if (AUR_Character* Character = GetGameCharacterFromActorInfo())
    {
        if (Character->IsLocallyControlled() && Character->bPressedJump)
        {
            Character->StopJumping();
        }
    }
}
