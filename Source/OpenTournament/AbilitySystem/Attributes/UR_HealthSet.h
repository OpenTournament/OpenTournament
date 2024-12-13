// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "AbilitySystemComponent.h"
#include "NativeGameplayTags.h"
#include "UR_AttributeSet.h"

#include "UR_HealthSet.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_Damage);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_DamageImmunity);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_DamageSelfDestruct);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_FellOutOfWorld);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Game_Damage_Message);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_Health_Change);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_MaxHealth_Change);

/////////////////////////////////////////////////////////////////////////////////////////////////

class UObject;
struct FFrame;
struct FGameplayEffectModCallbackData;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * UUR_HealthSet
 *
 *	Class that defines attributes that are necessary for taking damage.
 *	Attribute examples include: health, shields, and resistances.
 */
UCLASS(BlueprintType)
class UUR_HealthSet : public UUR_AttributeSet
{
    GENERATED_BODY()

public:
    UUR_HealthSet();

    ATTRIBUTE_ACCESSORS(UUR_HealthSet, Health);
    ATTRIBUTE_ACCESSORS(UUR_HealthSet, MaxHealth);
    ATTRIBUTE_ACCESSORS(UUR_HealthSet, Healing);
    ATTRIBUTE_ACCESSORS(UUR_HealthSet, Damage);

    // Delegate when health changes due to damage/healing, some information may be missing on the client
    mutable FUR_AttributeEvent OnHealthChanged;

    // Delegate when max health changes
    mutable FUR_AttributeEvent OnMaxHealthChanged;

    // Delegate to broadcast when the health attribute reaches zero
    mutable FUR_AttributeEvent OnOutOfHealth;

protected:
    UFUNCTION()
    void OnRep_Health(const FGameplayAttributeData& OldValue);

    UFUNCTION()
    void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

    virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

    virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
    virtual void PostAttributeBaseChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) const override;
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

    void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;

private:
    // The current health attribute.  The health will be capped by the max health attribute.  Health is hidden from modifiers so only executions can modify it.
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Game|Health", Meta = (HideFromModifiers, AllowPrivateAccess = true))
    FGameplayAttributeData Health;

    // The current max health attribute.  Max health is an attribute since gameplay effects can modify it.
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Game|Health", Meta = (AllowPrivateAccess = true))
    FGameplayAttributeData MaxHealth;

    // Used to track when the health reaches 0.
    bool bOutOfHealth;

    // Store the health before any changes
    float MaxHealthBeforeAttributeChange;
    float HealthBeforeAttributeChange;

    // -------------------------------------------------------------------
    //	Meta Attribute (please keep attributes that aren't 'stateful' below
    // -------------------------------------------------------------------

    // Incoming healing. This is mapped directly to +Health
    UPROPERTY(BlueprintReadOnly, Category="Game|Health", Meta=(AllowPrivateAccess=true))
    FGameplayAttributeData Healing;

    // Incoming damage. This is mapped directly to -Health
    UPROPERTY(BlueprintReadOnly, Category="Game|Health", Meta=(HideFromModifiers, AllowPrivateAccess=true))
    FGameplayAttributeData Damage;
};
