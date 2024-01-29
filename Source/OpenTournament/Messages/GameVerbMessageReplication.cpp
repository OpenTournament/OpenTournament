// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "GameVerbMessageReplication.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "Messages/GameVerbMessage.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameVerbMessageReplication)

//////////////////////////////////////////////////////////////////////
// FGameVerbMessageReplicationEntry

FString FGameVerbMessageReplicationEntry::GetDebugString() const
{
	return Message.ToString();
}

//////////////////////////////////////////////////////////////////////
// FGameVerbMessageReplication

void FGameVerbMessageReplication::AddMessage(const FGameVerbMessage& Message)
{
	FGameVerbMessageReplicationEntry& NewStack = CurrentMessages.Emplace_GetRef(Message);
	MarkItemDirty(NewStack);
}

void FGameVerbMessageReplication::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
// 	for (int32 Index : RemovedIndices)
// 	{
// 		const FGameplayTag Tag = CurrentMessages[Index].Tag;
// 		TagToCountMap.Remove(Tag);
// 	}
}

void FGameVerbMessageReplication::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		const FGameVerbMessageReplicationEntry& Entry = CurrentMessages[Index];
		RebroadcastMessage(Entry.Message);
	}
}

void FGameVerbMessageReplication::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		const FGameVerbMessageReplicationEntry& Entry = CurrentMessages[Index];
		RebroadcastMessage(Entry.Message);
	}
}

void FGameVerbMessageReplication::RebroadcastMessage(const FGameVerbMessage& Message)
{
	check(Owner);
	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(Owner);
	MessageSystem.BroadcastMessage(Message.Verb, Message);
}

