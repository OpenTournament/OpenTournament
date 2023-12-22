// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Components/ActorComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"

#include "GameplayMessageProcessor.generated.h"

namespace EEndPlayReason { enum Type : int; }

class UObject;

/**
 * UGameplayMessageProcessor
 *
 * Base class for any message processor which observes other gameplay messages
 * and potentially re-emits updates (e.g., when a chain or combo is detected)
 *
 * Note that these processors are spawned on the server once (not per player)
 * and should do their own internal filtering if only relevant for some players.
 */
UCLASS(BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class OPENTOURNAMENT_API UGameplayMessageProcessor : public UActorComponent
{
	GENERATED_BODY()

public:
	//~UActorComponent interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of UActorComponent interface

	virtual void StartListening();
	virtual void StopListening();

protected:
	void AddListenerHandle(FGameplayMessageListenerHandle&& Handle);
	double GetServerTime() const;

private:
	TArray<FGameplayMessageListenerHandle> ListenerHandles;
};
