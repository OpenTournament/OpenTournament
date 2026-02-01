// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UR_GameplayMessageProcessor.h"

#include "UR_EliminationStreakProcessor.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class APlayerState;
class UObject;
struct FGameplayTag;
struct FGameVerbMessage;
template <typename T> struct TObjectPtr;

/////////////////////////////////////////////////////////////////////////////////////////////////

// Tracks a streak of eliminations (X eliminations without being eliminated)
UCLASS(Abstract)
class UUR_EliminationStreakProcessor : public UUR_GameplayMessageProcessor
{
	GENERATED_BODY()

public:
	virtual void StartListening() override;

protected:
	// The event to rebroadcast when a user gets a streak of a certain length
	UPROPERTY(EditDefaultsOnly)
	TMap<int32, FGameplayTag> ElimStreakTags;

private:
	void OnEliminationMessage(FGameplayTag Channel, const FGameVerbMessage& Payload);

private:
	UPROPERTY(Transient)
	TMap<TObjectPtr<APlayerState>, int32> PlayerStreakHistory;
};
