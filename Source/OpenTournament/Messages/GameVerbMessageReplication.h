// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameplayTagContainer.h"
#include "GameVerbMessage.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "GameVerbMessageReplication.generated.h"

class UObject;
struct FGameVerbMessageReplication;
struct FNetDeltaSerializeInfo;

/**
 * Represents one verb message
 */
USTRUCT(BlueprintType)
struct FGameVerbMessageReplicationEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FGameVerbMessageReplicationEntry()
	{}

	FGameVerbMessageReplicationEntry(const FGameVerbMessage& InMessage)
		: Message(InMessage)
	{
	}

	FString GetDebugString() const;

private:
	friend FGameVerbMessageReplication;

	UPROPERTY()
	FGameVerbMessage Message;
};

/** Container of verb messages to replicate */
USTRUCT(BlueprintType)
struct FGameVerbMessageReplication : public FFastArraySerializer
{
	GENERATED_BODY()

	FGameVerbMessageReplication()
	{
	}

public:
	void SetOwner(UObject* InOwner) { Owner = InOwner; }

	// Broadcasts a message from server to clients
	void AddMessage(const FGameVerbMessage& Message);

	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FGameVerbMessageReplicationEntry, FGameVerbMessageReplication>(CurrentMessages, DeltaParms, *this);
	}

private:
	void RebroadcastMessage(const FGameVerbMessage& Message);

private:
	// Replicated list of gameplay tag stacks
	UPROPERTY()
	TArray<FGameVerbMessageReplicationEntry> CurrentMessages;

	// Owner (for a route to a world)
	UPROPERTY()
	TObjectPtr<UObject> Owner = nullptr;
};

template<>
struct TStructOpsTypeTraits<FGameVerbMessageReplication> : public TStructOpsTypeTraitsBase2<FGameVerbMessageReplication>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
