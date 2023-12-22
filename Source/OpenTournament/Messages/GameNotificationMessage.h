// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "NativeGameplayTags.h"

#include "GameNotificationMessage.generated.h"

class UObject;

OPENTOURNAMENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Game_AddNotification_Message);

class APlayerState;

// A message destined for a transient log (e.g., an elimination feed or inventory pickup stream)
USTRUCT(BlueprintType)
struct OPENTOURNAMENT_API FGameNotificationMessage
{
	GENERATED_BODY()

	// The destination channel
	UPROPERTY(BlueprintReadWrite, Category=Notification)
	FGameplayTag TargetChannel;

	// The target player (if none is set then it will display for all local players)
	UPROPERTY(BlueprintReadWrite, Category=Notification)
	TObjectPtr<APlayerState> TargetPlayer = nullptr;

	// The message to display
	UPROPERTY(BlueprintReadWrite, Category=Notification)
	FText PayloadMessage;

	// Extra payload specific to the target channel (e.g., a style or definition asset)
	UPROPERTY(BlueprintReadWrite, Category=Notification)
	FGameplayTag PayloadTag;

	// Extra payload specific to the target channel (e.g., a style or definition asset)
	UPROPERTY(BlueprintReadWrite, Category=Notification)
	TObjectPtr<UObject> PayloadObject = nullptr;
};
