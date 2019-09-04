// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LocalPlayer.h"
#include "UR_LocalPlayer.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

enum class EChatChannel : uint8;

USTRUCT(BlueprintType)
struct FChatHistoryEntry
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly)
	FDateTime Time;

	UPROPERTY(BlueprintReadOnly)
	FString SenderName;

	UPROPERTY(BlueprintReadOnly)
	FString Message;

	UPROPERTY(BlueprintReadOnly)
	EChatChannel Channel;

	/**
	* Team index of message author, when we received the message.
	* This is necessary so we can build accurate history, even after PlayerStates are no longer available.
	* -1 is used for spectators
	* -2 is used for other entites (not associated with a player state)
	*/
	UPROPERTY(BlueprintReadOnly)
	int32 SenderTeamIdx;

	/**
	* Team index of the message itself.
	* This is only relevant when channel is TEAM.
	* Same reasons as above.
	*/
	UPROPERTY(BlueprintReadOnly)
	int32 MessageTeamIdx;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

#define CHAT_HISTORY_MAX 1000

/**
 * 
 */
UCLASS(BlueprintType)
class OPENTOURNAMENT_API UUR_LocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()

	UUR_LocalPlayer(const FObjectInitializer& ObjectInitializer);

public:

	/**
	* Persistent chat history
	*/
	UPROPERTY(BlueprintReadOnly)
	TArray<FChatHistoryEntry> ChatHistory;
};
