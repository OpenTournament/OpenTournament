// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UR_GameplayMessageProcessor.h"

#include "UR_EliminationChordProcessor.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class APlayerState;
class UObject;
struct FGameplayTag;
struct FGameVerbMessage;
template <typename T> struct TObjectPtr;

/////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct FPlayerEliminationChordInfo
{
	GENERATED_BODY()

	double LastEliminationTime = 0.0;

	int32 ChordCounter = 1;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

// Tracks a chord of eliminations (X eliminations without more than Y seconds passing between each one)
UCLASS(Abstract)
class UUR_EliminationChordProcessor : public UUR_GameplayMessageProcessor
{
	GENERATED_BODY()

public:
	virtual void StartListening() override;

protected:
	UPROPERTY(EditDefaultsOnly)
	float ChordTimeLimit = 4.5f;

	// The event to rebroadcast when a user gets a chord of a certain length
	UPROPERTY(EditDefaultsOnly)
	TMap<int32, FGameplayTag> EliminationChordTags;

private:
	void OnEliminationMessage(FGameplayTag Channel, const FGameVerbMessage& Payload);

private:
	UPROPERTY(Transient)
	TMap<TObjectPtr<APlayerState>, FPlayerEliminationChordInfo> PlayerChordHistory;
};
