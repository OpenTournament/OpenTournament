// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UR_GameplayMessageProcessor.h"

#include "UR_AssistProcessor.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class APlayerState;
class UObject;
struct FGameplayTag;
struct FGameVerbMessage;
template <typename T>
struct TObjectPtr;

/////////////////////////////////////////////////////////////////////////////////////////////////

// Tracks the damage done to a player by other players
USTRUCT()
struct FPlayerAssistDamageTracking
{
    GENERATED_BODY()

    // Map of damager to damage dealt
    UPROPERTY(Transient)
    TMap<TObjectPtr<APlayerState>, float> AccumulatedDamageByPlayer;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

// Tracks assists (dealing damage to another player without finishing them)
UCLASS()
class UUR_AssistProcessor : public UUR_GameplayMessageProcessor
{
    GENERATED_BODY()

public:
    virtual void StartListening() override;

private:
    void OnDamageMessage(FGameplayTag Channel, const FGameVerbMessage& Payload);
    void OnEliminationMessage(FGameplayTag Channel, const FGameVerbMessage& Payload);

private:
    // Map of player to damage dealt to them
    UPROPERTY(Transient)
    TMap<TObjectPtr<APlayerState>, FPlayerAssistDamageTracking> DamageHistory;
};
