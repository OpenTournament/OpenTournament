// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Interaction/Tasks/AbilityTask_WaitForInteractableTargets.h"

#include "Interaction/InteractionQuery.h"

#include "AbilityTask_WaitForInteractableTargets_SingleLineTrace.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

struct FCollisionProfileName;

class UGameplayAbility;
class UObject;
struct FFrame;

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class UAbilityTask_WaitForInteractableTargets_SingleLineTrace : public UAbilityTask_WaitForInteractableTargets
{
    GENERATED_BODY()

public:
    UAbilityTask_WaitForInteractableTargets_SingleLineTrace(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    virtual void Activate() override;

    /** Wait until we trace new set of interactables.  This task automatically loops. */
    UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
    static UAbilityTask_WaitForInteractableTargets_SingleLineTrace* WaitForInteractableTargets_SingleLineTrace(UGameplayAbility* OwningAbility, FInteractionQuery InteractionQuery, FCollisionProfileName TraceProfile, FGameplayAbilityTargetingLocationInfo StartLocation, float InteractionScanRange = 100, float InteractionScanRate = 0.100, bool bShowDebug = false);

private:
    virtual void OnDestroy(bool AbilityEnded) override;

    void PerformTrace();

    UPROPERTY()
    FInteractionQuery InteractionQuery;

    UPROPERTY()
    FGameplayAbilityTargetingLocationInfo StartLocation;

    float InteractionScanRange = 100;
    float InteractionScanRate = 0.100;
    bool bShowDebug = false;

    FTimerHandle TimerHandle;
};
