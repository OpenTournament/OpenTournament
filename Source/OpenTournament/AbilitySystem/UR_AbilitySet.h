// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ActiveGameplayEffectHandle.h"
#include "AttributeSet.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"

#include "UR_AbilitySet.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UAttributeSet;
class UGameplayEffect;
class UUR_AbilitySystemComponent;
class UUR_GameplayAbility;
class UObject;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * FUR_AbilitySet_GameplayAbility
 *
 *	Data used by the ability set to grant gameplay abilities.
 */
USTRUCT(BlueprintType)
struct FUR_AbilitySet_GameplayAbility
{
    GENERATED_BODY()

public:
    // Gameplay ability to grant.
    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<UUR_GameplayAbility> Ability = nullptr;

    // Level of ability to grant.
    UPROPERTY(EditDefaultsOnly)
    int32 AbilityLevel = 1;

    // Tag used to process input for the ability.
    UPROPERTY(EditDefaultsOnly, Meta = (Categories = "InputTag"))
    FGameplayTag InputTag;
};


/**
 * FUR_AbilitySet_GameplayEffect
 *
 *	Data used by the ability set to grant gameplay effects.
 */
USTRUCT(BlueprintType)
struct FUR_AbilitySet_GameplayEffect
{
    GENERATED_BODY()

public:
    // Gameplay effect to grant.
    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<UGameplayEffect> GameplayEffect = nullptr;

    // Level of gameplay effect to grant.
    UPROPERTY(EditDefaultsOnly)
    float EffectLevel = 1.0f;
};

/**
 * FUR_AbilitySet_AttributeSet
 *
 *	Data used by the ability set to grant attribute sets.
 */
USTRUCT(BlueprintType)
struct FUR_AbilitySet_AttributeSet
{
    GENERATED_BODY()

public:
    // Gameplay effect to grant.
    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<UAttributeSet> AttributeSet;
};

/**
 * FUR_AbilitySet_GrantedHandles
 *
 *	Data used to store handles to what has been granted by the ability set.
 */
USTRUCT(BlueprintType)
struct FUR_AbilitySet_GrantedHandles
{
    GENERATED_BODY()

public:
    void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);

    void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);

    void AddAttributeSet(UAttributeSet* Set);

    void TakeFromAbilitySystem(UUR_AbilitySystemComponent* InASC);

protected:
    // Handles to the granted abilities.
    UPROPERTY()
    TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

    // Handles to the granted gameplay effects.
    UPROPERTY()
    TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;

    // Pointers to the granted attribute sets
    UPROPERTY()
    TArray<TObjectPtr<UAttributeSet>> GrantedAttributeSets;
};


/**
 * UUR_AbilitySet
 *
 *	Non-mutable data asset used to grant gameplay abilities and gameplay effects.
 */
UCLASS(BlueprintType, Const)
class UUR_AbilitySet : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UUR_AbilitySet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    // Grants the ability set to the specified ability system component.
    // The returned handles can be used later to take away anything that was granted.
    void GiveToAbilitySystem(UUR_AbilitySystemComponent* InASC, FUR_AbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject = nullptr) const;

protected:
    // Gameplay abilities to grant when this ability set is granted.
    UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities", meta=(TitleProperty=Ability))
    TArray<FUR_AbilitySet_GameplayAbility> GrantedGameplayAbilities;

    // Gameplay effects to grant when this ability set is granted.
    UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects", meta=(TitleProperty=GameplayEffect))
    TArray<FUR_AbilitySet_GameplayEffect> GrantedGameplayEffects;

    // Attribute sets to grant when this ability set is granted.
    UPROPERTY(EditDefaultsOnly, Category = "Attribute Sets", meta=(TitleProperty=AttributeSet))
    TArray<FUR_AbilitySet_AttributeSet> GrantedAttributes;
};
