// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"

#include "AbilitySystemComponent.h"

#include "OpenTournament.h"
#include "UR_AttributeSet.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/////////////////////////////////////////////////////////////////////////////////////////////////

class UUR_AbilitySystemComponent;
/**
 * Delegate used to broadcast attribute events, some of these parameters may be null on clients:
 * @param EffectInstigator	The original instigating actor for this event
 * @param EffectCauser		The physical actor that caused the change
 * @param EffectSpec		The full effect spec for this change
 * @param EffectMagnitude	The raw magnitude, this is before clamping
 * @param OldValue			The value of the attribute before it was changed
 * @param NewValue			The value after it was changed
*/
DECLARE_MULTICAST_DELEGATE_SixParams(FUR_AttributeEvent,
    AActor* /*EffectInstigator*/,
    AActor* /*EffectCauser*/,
    const FGameplayEffectSpec* /*EffectSpec*/,
    float /*EffectMagnitude*/,
    float /*OldValue*/,
    float /*NewValue*/);

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
* Character - Primary Attribute Set
*/
UCLASS()
class OPENTOURNAMENT_API UUR_AttributeSet : public UAttributeSet
{
    GENERATED_BODY()

public:
    UUR_AttributeSet();
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    UPROPERTY(ReplicatedUsing=OnRep_HealthD, BlueprintReadWrite, EditInstanceOnly, Category = "CharacterAttributes")
    FGameplayAttributeData Health_D;
    ATTRIBUTE_ACCESSORS(UUR_AttributeSet, Health_D)

    UPROPERTY(ReplicatedUsing=OnRep_HealthMaxD, BlueprintReadWrite, EditInstanceOnly, Category = "CharacterAttributes")
    FGameplayAttributeData HealthMax_D;
    ATTRIBUTE_ACCESSORS(UUR_AttributeSet, HealthMax_D)

    UPROPERTY(ReplicatedUsing=OnRep_OverHealth, BlueprintReadWrite, EditInstanceOnly, Category = "CharacterAttributes")
    FGameplayAttributeData OverHealth;
    ATTRIBUTE_ACCESSORS(UUR_AttributeSet, OverHealth)

    UPROPERTY(ReplicatedUsing=OnRep_OverHealthMax, BlueprintReadWrite, EditInstanceOnly, Category = "CharacterAttributes")
    FGameplayAttributeData OverHealthMax;
    ATTRIBUTE_ACCESSORS(UUR_AttributeSet, OverHealthMax)

    ////////========////////

    UPROPERTY(ReplicatedUsing=OnRep_Armor, BlueprintReadWrite, EditInstanceOnly, Category = "CharacterAttributes")
    FGameplayAttributeData Armor;
    ATTRIBUTE_ACCESSORS(UUR_AttributeSet, Armor)

    UPROPERTY(ReplicatedUsing=OnRep_ArmorMax, BlueprintReadWrite, EditInstanceOnly, Category = "CharacterAttributes")
    FGameplayAttributeData ArmorMax;
    ATTRIBUTE_ACCESSORS(UUR_AttributeSet, ArmorMax)

    UPROPERTY(ReplicatedUsing=OnRep_ArmorAbsorptionPercent, BlueprintReadWrite, EditInstanceOnly, Category = "CharacterAttributes")
    FGameplayAttributeData ArmorAbsorptionPercent;
    ATTRIBUTE_ACCESSORS(UUR_AttributeSet, ArmorAbsorptionPercent)

    ////////========////////

    UPROPERTY(ReplicatedUsing=OnRep_Shield, BlueprintReadWrite, EditInstanceOnly, Category = "CharacterAttributes")
    FGameplayAttributeData Shield;
    ATTRIBUTE_ACCESSORS(UUR_AttributeSet, Shield)

    UPROPERTY(ReplicatedUsing=OnRep_ShieldMax, BlueprintReadWrite, EditInstanceOnly, Category = "CharacterAttributes")
    FGameplayAttributeData ShieldMax;
    ATTRIBUTE_ACCESSORS(UUR_AttributeSet, ShieldMax)

    ////////========////////

    UPROPERTY(ReplicatedUsing=OnRep_Energy, BlueprintReadWrite, EditInstanceOnly, Category = "CharacterAttributes")
    FGameplayAttributeData Energy;
    ATTRIBUTE_ACCESSORS(UUR_AttributeSet, Energy)

    UPROPERTY(ReplicatedUsing=OnRep_EnergyMax, BlueprintReadWrite, EditInstanceOnly, Category = "CharacterAttributes")
    FGameplayAttributeData EnergyMax;
    ATTRIBUTE_ACCESSORS(UUR_AttributeSet, EnergyMax)

    ////////========////////

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "CharacterAttributes")
    FGameplayAttributeData DamageScale;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "CharacterAttributes")
    FGameplayAttributeData FireRate;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "CharacterAttributes")
    FGameplayAttributeData ReloadRate;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "CharacterAttributes")
    FGameplayAttributeData GroundSpeed;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "CharacterAttributes")
    FGameplayAttributeData MovementAcceleration;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "CharacterAttributes")
    FGameplayAttributeData CrouchedSpeed;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "CharacterAttributes")
    FGameplayAttributeData SwimSpeed;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "CharacterAttributes")
    FGameplayAttributeData JumpVelocity;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "CharacterAttributes")
    FGameplayAttributeData AirControl;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "CharacterAttributes")
    FGameplayAttributeData DodgeImpulseHorizontal;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "CharacterAttributes")
    FGameplayAttributeData DodgeImpulseVertical;

    //-------------------------------------------------------------------------------------//

    //NOTE: Not sure how to best implement this within the GAS framework...
    UFUNCTION(BlueprintPure, Category = "CharacterAttributes")
    virtual float GetEffectiveHealth();

protected:
    /**
    * Helper function to proportionally adjust the value of an attribute when it's associated max attribute changes.
    * (i.e. When MaxHealth increases, Health increases by an amount that maintains the same percentage as before)
    */
    void AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty);

    // These OnRep functions exist to make sure that the ability system internal representations are synchronized properly during replication
    UFUNCTION()
    virtual void OnRep_HealthD(const FGameplayAttributeData& OldHealth);

    UFUNCTION()
    virtual void OnRep_HealthMaxD(const FGameplayAttributeData& OldHealthMax);

    UFUNCTION()
    virtual void OnRep_OverHealth(const FGameplayAttributeData& OldOverHealth);

    UFUNCTION()
    virtual void OnRep_OverHealthMax(const FGameplayAttributeData& OldOverHealthMax);

    UFUNCTION()
    virtual void OnRep_Energy(const FGameplayAttributeData& OldEnergy);

    UFUNCTION()
    virtual void OnRep_EnergyMax(const FGameplayAttributeData& OldEnergyMax);

    UFUNCTION()
    virtual void OnRep_Armor(const FGameplayAttributeData& OldArmor);

    UFUNCTION()
    virtual void OnRep_ArmorMax(const FGameplayAttributeData& OldArmorMax);

    UFUNCTION()
    virtual void OnRep_ArmorAbsorptionPercent(const FGameplayAttributeData& OldArmorAbsorptionPercent);

    UFUNCTION()
    virtual void OnRep_Shield(const FGameplayAttributeData& OldShield);

    UFUNCTION()
    virtual void OnRep_ShieldMax(const FGameplayAttributeData& OldShieldMax);

    UUR_AbilitySystemComponent* GetGameAbilitySystemComponent() const;
};
