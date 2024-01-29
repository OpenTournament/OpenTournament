// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_AbilitySystemComponent.h"

#include "AbilitySystemGlobals.h"

#include "UR_Character.h"
#include "UR_GameplayAbility.h"
#include "UR_AttributeSet.h"

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

const UUR_AttributeSet* UUR_AbilitySystemComponent::GetURAttributeSetFromActor(const AActor* Actor, bool LookForComponent)
{
    if (auto ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor, LookForComponent))
    {
        return ASC->GetSet<UUR_AttributeSet>();
    }
    return NULL;
}
