// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Abilities/Tasks/AbilityTask.h"

#include "Engine/CollisionProfile.h"
#include "Interaction/InteractionOption.h"

#include "AbilityTask_WaitForInteractableTargets.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AActor;
class IInteractableTarget;
class UObject;
class UWorld;
struct FCollisionQueryParams;
struct FHitResult;
struct FInteractionQuery;
template <typename InterfaceType>
class TScriptInterface;

/////////////////////////////////////////////////////////////////////////////////////////////////

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractableObjectsChangedEvent, const TArray<FInteractionOption>&, InteractableOptions);

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(Abstract)
class UAbilityTask_WaitForInteractableTargets : public UAbilityTask
{
    GENERATED_BODY()

public:
    UAbilityTask_WaitForInteractableTargets(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    UPROPERTY(BlueprintAssignable)
    FInteractableObjectsChangedEvent InteractableObjectsChanged;

protected:
    static void LineTrace(FHitResult& OutHitResult, const UWorld* World, const FVector& Start, const FVector& End, FName ProfileName, const FCollisionQueryParams Params);

    void AimWithPlayerController(const AActor* InSourceActor, FCollisionQueryParams Params, const FVector& TraceStart, float MaxRange, FVector& OutTraceEnd, bool bIgnorePitch = false) const;

    static bool ClipCameraRayToAbilityRange(FVector CameraLocation, FVector CameraDirection, FVector AbilityCenter, float AbilityRange, FVector& ClippedPosition);

    void UpdateInteractableOptions(const FInteractionQuery& InteractQuery, const TArray<TScriptInterface<IInteractableTarget>>& InteractableTargets);

    FCollisionProfileName TraceProfile;

    // Does the trace affect the aiming pitch
    bool bTraceAffectsAimPitch = true;

    TArray<FInteractionOption> CurrentOptions;
};
