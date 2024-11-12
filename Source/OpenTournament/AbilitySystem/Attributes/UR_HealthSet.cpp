// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_HealthSet.h"

#include "GameplayEffectExtension.h"
#include "Engine/World.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Net/UnrealNetwork.h"

#include "UR_AbilitySystemComponent.h"
#include "UR_AttributeSet.h"
#include "UR_GameplayTags.h"
#include "Messages/GameVerbMessage.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_HealthSet)

/////////////////////////////////////////////////////////////////////////////////////////////////

UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_Damage, "Gameplay.Damage");
UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_DamageImmunity, "Gameplay.DamageImmunity");
UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_DamageSelfDestruct, "Gameplay.Damage.SelfDestruct");
UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_FellOutOfWorld, "Gameplay.Damage.FellOutOfWorld");
UE_DEFINE_GAMEPLAY_TAG(TAG_Game_Damage_Message, "Game.Damage.Message");
UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_Health_Change, "Gameplay.Health.Change");
UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_MaxHealth_Change, "Gameplay.MaxHealth.Change");

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_HealthSet::UUR_HealthSet()
	: Health(100.0f)
	, MaxHealth(100.0f)
{
    bOutOfHealth = false;
    MaxHealthBeforeAttributeChange = 0.0f;
    HealthBeforeAttributeChange = 0.0f;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_HealthSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(UUR_HealthSet, Health, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UUR_HealthSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_HealthSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UUR_HealthSet, Health, OldValue);

    // Call the change callback, but without an instigator
    // This could be changed to an explicit RPC in the future
    // These events on the client should not be changing attributes

    const float CurrentHealth = GetHealth();
    const float EstimatedMagnitude = CurrentHealth - OldValue.GetCurrentValue();

    OnHealthChanged.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentHealth);

    if (!bOutOfHealth && CurrentHealth <= 0.0f)
    {
        OnOutOfHealth.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentHealth);
    }

    bOutOfHealth = (CurrentHealth <= 0.0f);
}

void UUR_HealthSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UUR_HealthSet, MaxHealth, OldValue);

    // Call the change callback, but without an instigator
    // This could be changed to an explicit RPC in the future
    OnMaxHealthChanged.Broadcast(nullptr, nullptr, nullptr, GetMaxHealth() - OldValue.GetCurrentValue(), OldValue.GetCurrentValue(), GetMaxHealth());
}

bool UUR_HealthSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
    if (!Super::PreGameplayEffectExecute(Data))
    {
        return false;
    }

    // Handle modifying incoming normal damage
    if (Data.EvaluatedData.Attribute == GetDamageAttribute())
    {
        if (Data.EvaluatedData.Magnitude > 0.0f)
        {
            const bool bIsDamageFromSelfDestruct = Data.EffectSpec.GetDynamicAssetTags().HasTagExact(TAG_Gameplay_DamageSelfDestruct);

            if (Data.Target.HasMatchingGameplayTag(TAG_Gameplay_DamageImmunity) && !bIsDamageFromSelfDestruct)
            {
                // Do not take away any health.
                Data.EvaluatedData.Magnitude = 0.0f;
                return false;
            }

#if !UE_BUILD_SHIPPING
            // Check GodMode cheat, unlimited health is checked below
            if (Data.Target.HasMatchingGameplayTag(URGameplayTags::Cheat_GodMode) && !bIsDamageFromSelfDestruct)
            {
                // Do not take away any health.
                Data.EvaluatedData.Magnitude = 0.0f;
                return false;
            }
#endif // #if !UE_BUILD_SHIPPING
        }
    }

    // Save the current health
    HealthBeforeAttributeChange = GetHealth();
    MaxHealthBeforeAttributeChange = GetMaxHealth();

    return true;
}

void UUR_HealthSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    const bool bIsDamageFromSelfDestruct = Data.EffectSpec.GetDynamicAssetTags().HasTagExact(TAG_Gameplay_DamageSelfDestruct);
    float MinimumHealth = 0.0f;

#if !UE_BUILD_SHIPPING
    // Godmode and unlimited health stop death unless it's a self destruct
    if (!bIsDamageFromSelfDestruct &&
        (Data.Target.HasMatchingGameplayTag(URGameplayTags::Cheat_GodMode) || Data.Target.HasMatchingGameplayTag(URGameplayTags::Cheat_UnlimitedHealth)))
    {
        MinimumHealth = 1.0f;
    }
#endif // #if !UE_BUILD_SHIPPING

    const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
    AActor* Instigator = EffectContext.GetOriginalInstigator();
    AActor* Causer = EffectContext.GetEffectCauser();

    if (Data.EvaluatedData.Attribute == GetDamageAttribute())
    {
        // Send a standardized verb message that other systems can observe
        if (Data.EvaluatedData.Magnitude > 0.0f)
        {
            FGameVerbMessage Message;
            Message.Verb = TAG_Game_Damage_Message;
            Message.Instigator = Data.EffectSpec.GetEffectContext().GetEffectCauser();
            Message.InstigatorTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
            Message.Target = GetOwningActor();
            Message.TargetTags = *Data.EffectSpec.CapturedTargetTags.GetAggregatedTags();
            //@TODO: Fill out context tags, and any non-ability-system source/instigator tags
            //@TODO: Determine if it's an opposing team kill, self-own, team kill, etc...
            Message.Magnitude = Data.EvaluatedData.Magnitude;

            UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
            MessageSystem.BroadcastMessage(Message.Verb, Message);
        }

        // Convert into -Health and then clamp
        SetHealth(FMath::Clamp(GetHealth() - GetDamage(), MinimumHealth, GetMaxHealth()));
        SetDamage(0.0f);
    }
    else if (Data.EvaluatedData.Attribute == GetHealingAttribute())
    {
        // Convert into +Health and then clamo
        SetHealth(FMath::Clamp(GetHealth() + GetHealing(), MinimumHealth, GetMaxHealth()));
        SetHealing(0.0f);
    }
    else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        // Clamp and fall into out of health handling below
        SetHealth(FMath::Clamp(GetHealth(), MinimumHealth, GetMaxHealth()));
    }
    else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
    {
        // TODO clamp current health?

        // Notify on any requested max health changes
        OnMaxHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, MaxHealthBeforeAttributeChange, GetMaxHealth());

        FGameVerbMessage Message;
        Message.Verb = TAG_Gameplay_MaxHealth_Change;
        Message.Instigator = Data.EffectSpec.GetEffectContext().GetEffectCauser();
        Message.InstigatorTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
        Message.Target =  Data.Target;
        Message.TargetTags = *Data.EffectSpec.CapturedTargetTags.GetAggregatedTags();
        Message.Magnitude = GetMaxHealth();

        UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
        MessageSystem.BroadcastMessage(Message.Verb, Message);
    }

    // If health has actually changed activate callbacks
    const float CurrentHealth = GetHealth();
    if (CurrentHealth != HealthBeforeAttributeChange)
    {
        OnHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, HealthBeforeAttributeChange, GetHealth());

        FGameVerbMessage Message;
        Message.Verb = TAG_Gameplay_Health_Change;
        Message.Instigator = Data.EffectSpec.GetEffectContext().GetEffectCauser();
        Message.InstigatorTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
        Message.Target =  Data.Target;
        Message.TargetTags = *Data.EffectSpec.CapturedTargetTags.GetAggregatedTags();
        Message.Magnitude = CurrentHealth;

        UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
        MessageSystem.BroadcastMessage(Message.Verb, Message);
    }

    if ((CurrentHealth <= 0.0f) && !bOutOfHealth)
    {
        OnOutOfHealth.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, HealthBeforeAttributeChange, GetHealth());
    }

    // Check health again in case an event above changed it.
    bOutOfHealth = (CurrentHealth <= 0.0f);
}

void UUR_HealthSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
    Super::PreAttributeBaseChange(Attribute, NewValue);

    ClampAttribute(Attribute, NewValue);
}

void UUR_HealthSet::PostAttributeBaseChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) const
{
    Super::PostAttributeBaseChange(Attribute, OldValue, NewValue);

    if(Attribute == GetMaxHealthAttribute())
    {
        FGameVerbMessage Message;
        Message.Verb = TAG_Gameplay_MaxHealth_Change;
        Message.Magnitude = GetMaxHealth();

        UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
        MessageSystem.BroadcastMessage(Message.Verb, Message);
    }
    else if(Attribute == GetHealthAttribute())
    {
        FGameVerbMessage Message;
        Message.Verb = TAG_Gameplay_Health_Change;
        Message.Magnitude = GetMaxHealth();

        UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
        MessageSystem.BroadcastMessage(Message.Verb, Message);
    }
}

void UUR_HealthSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

    ClampAttribute(Attribute, NewValue);
}

void UUR_HealthSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    Super::PostAttributeChange(Attribute, OldValue, NewValue);

    if (Attribute == GetMaxHealthAttribute())
    {
        // Make sure current health is not greater than the new max health.
        if (GetHealth() > NewValue)
        {
            UUR_AbilitySystemComponent* ASC = GetGameAbilitySystemComponent();
            check(ASC);

            ASC->ApplyModToAttribute(GetHealthAttribute(), EGameplayModOp::Override, NewValue);
        }
    }

    if (bOutOfHealth && (GetHealth() > 0.0f))
    {
        bOutOfHealth = false;
    }
}

void UUR_HealthSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
    if (Attribute == GetHealthAttribute())
    {
        // Do not allow health to go negative or above max health.
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
    }
    else if (Attribute == GetMaxHealthAttribute())
    {
        // Do not allow max health to drop below 1.
        NewValue = FMath::Max(NewValue, 1.0f);
    }
}
