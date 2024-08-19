// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "AbilitySystemComponent.h"

#include "UR_AbilitySystemComponent.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UUR_AbilityTagRelationshipMapping;

/////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Subclass of ability system component with game-specific data
 * Game-specific subclass to provide utility functions
 */
UCLASS()
class OPENTOURNAMENT_API UUR_AbilitySystemComponent : public UAbilitySystemComponent
{
    GENERATED_BODY()

public:
    // Constructors and overrides
    UUR_AbilitySystemComponent();

    /** Returns a list of currently active ability instances that match the tags */
    void GetActiveAbilitiesWithTags(const FGameplayTagContainer& GameplayTagContainer, TArray<UGameplayAbility*>& ActiveAbilities) const;

    /** Returns the default level used for ability activations, derived from the character */
    int32 GetDefaultAbilityLevel() const;

    /** Version of function in AbilitySystemGlobals that returns correct type */
    static UUR_AbilitySystemComponent* GetAbilitySystemComponentFromActor(const AActor* Actor, bool LookForComponent = false);

    void ClearAbilityInput();

    /////////////////////////////////////////////////////////////////////////////////////////////////

    // Uses a gameplay effect to add the specified dynamic granted tag.
    void AddDynamicTagGameplayEffect(const FGameplayTag& Tag);

    // Removes all active instances of the gameplay effect that was used to add the specified dynamic granted tag.
    void RemoveDynamicTagGameplayEffect(const FGameplayTag& Tag);

    /** Sets the current tag relationship mapping, if null it will clear it out */
    void SetTagRelationshipMapping(UUR_AbilityTagRelationshipMapping* NewMapping);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    // If set, this table is used to look up tag relationships for activate and cancel
    UPROPERTY()
    TObjectPtr<UUR_AbilityTagRelationshipMapping> TagRelationshipMapping;

    // Handles to abilities that had their input pressed this frame.
    TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

    // Handles to abilities that had their input released this frame.
    TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

    // Handles to abilities that have their input held.
    TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    UFUNCTION(BlueprintPure, meta = (DeprecatedFunction))
    static const class UUR_AttributeSet* GetURAttributeSetFromActor(const AActor* Actor, bool LookForComponent = false);
};
