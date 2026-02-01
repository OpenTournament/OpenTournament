// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Abilities/Tasks/AbilityTask.h"

#include "AbilityTask_GrantNearbyInteraction.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UGameplayAbility;
class UObject;
struct FFrame;
struct FGameplayAbilitySpecHandle;
struct FObjectKey;

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class UAbilityTask_GrantNearbyInteraction : public UAbilityTask
{
    GENERATED_BODY()

public:

    UAbilityTask_GrantNearbyInteraction(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    virtual void Activate() override;

    /** Wait until an overlap occurs. This will need to be better fleshed out so we can specify game specific collision requirements */
    UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
    static UAbilityTask_GrantNearbyInteraction* GrantAbilitiesForNearbyInteractors(UGameplayAbility* OwningAbility, float InteractionScanRange, float InteractionScanRate);

private:
    virtual void OnDestroy(bool AbilityEnded) override;

    void QueryInteractables();

    float InteractionScanRange = 100;
    float InteractionScanRate = 0.100;

    FTimerHandle QueryTimerHandle;

    TMap<FObjectKey, FGameplayAbilitySpecHandle> InteractionAbilityCache;
};
