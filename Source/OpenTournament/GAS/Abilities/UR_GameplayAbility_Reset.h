// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GAS/UR_GameplayAbility.h"

#include "UR_GameplayAbility_Reset.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AActor;
class UObject;
struct FGameplayAbilityActorInfo;
struct FGameplayEventData;

/////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FUR_PlayerResetMessage
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<AActor> OwnerPlayerState = nullptr;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * UUR_GameplayAbility_Reset
 *
 *	Gameplay ability used for handling quickly resetting the player back to initial spawn state.
 *	Ability is activated automatically via the "GameplayEvent.RequestReset" ability trigger tag (server only).
 */
UCLASS()
class OPENTOURNAMENT_API UUR_GameplayAbility_Reset : public UUR_GameplayAbility
{
	GENERATED_BODY()

public:
	UUR_GameplayAbility_Reset(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
