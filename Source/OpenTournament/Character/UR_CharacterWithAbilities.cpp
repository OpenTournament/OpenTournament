// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_CharacterWithAbilities.h"

#include "AbilitySystem//Attributes/UR_CombatSet.h"
#include "AbilitySystem/Attributes/UR_HealthSet.h"
#include "GAS/UR_AbilitySystemComponent.h"
#include "Async/TaskGraphInterfaces.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_CharacterWithAbilities)

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_CharacterWithAbilities::AUR_CharacterWithAbilities(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    //AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UUR_AbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
    //AbilitySystemComponent->SetIsReplicated(true);
    //AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    // These attribute sets will be detected by AbilitySystemComponent::InitializeComponent. Keeping a reference so that the sets don't get garbage collected before that.
    //HealthSet = CreateDefaultSubobject<UUR_HealthSet>(TEXT("HealthSet"));
    //CombatSet = CreateDefaultSubobject<UUR_CombatSet>(TEXT("CombatSet"));

    // AbilitySystemComponent needs to be updated at a high frequency.
    SetNetUpdateFrequency(100.0f);
}

void AUR_CharacterWithAbilities::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    check(AbilitySystemComponent);
    AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

UAbilitySystemComponent* AUR_CharacterWithAbilities::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}
