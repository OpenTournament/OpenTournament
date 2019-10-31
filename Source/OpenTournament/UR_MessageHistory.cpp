// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_MessageHistory.h"

#include "UR_FunctionLibrary.h"
#include "UR_ChatComponent.h"
#include "UR_PlayerState.h"

#include "Slate/UR_RichTextDecorator_CustomStyle.h"


DEFINE_LOG_CATEGORY(LogChat);
DEFINE_LOG_CATEGORY(LogMessages);


UUR_MessageHistory::UUR_MessageHistory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	History.Reserve(MESSAGES_HISTORY_MAX);
}


void UUR_MessageHistory::Append(const FMessageHistoryEntry& Entry)
{
	if (History.Num() >= MESSAGES_HISTORY_MAX)
		History.RemoveAt(0, 1, false);

	History.Emplace(Entry);

	OnNewMessageHistoryEntry.Broadcast(Entry);
}


void UUR_MessageHistory::OnReceiveChatMessage(const FString& SenderName, const FString& Message, int32 TeamIndex, APlayerState* SenderPS)
{
	FMessageHistoryEntry Entry;
	Entry.Time = FDateTime::Now();

	if (TeamIndex >= 0)
		Entry.Type = MessageType::TeamChat;
	else if (TeamIndex == CHAT_INDEX_SPEC)
		Entry.Type = MessageType::SpecChat;
	else
		Entry.Type = MessageType::GlobalChat;

	Entry.Parts = { SenderName, Message };
	Entry.Colors = {
		UUR_FunctionLibrary::GetPlayerDisplayTextColor(SenderPS),
		UUR_ChatComponent::GetChatMessageColor(this, TeamIndex),
	};

	Append(Entry);

	// log something for good measure
	UE_LOG(LogChat, Log, TEXT("%s%s : %s"), (TeamIndex >= 0) ? TEXT("[Team] ") : ((TeamIndex == CHAT_INDEX_SPEC) ? TEXT("[Spec] ") : TEXT("")), *SenderName, *Message);
}


void UUR_MessageHistory::OnReceiveSystemMessage(const FString& Message)
{
	FMessageHistoryEntry Entry;
	Entry.Time = FDateTime::Now();
	Entry.Type = MessageType::System;
	Entry.Parts = { Message };
	Entry.Colors = {};
	Append(Entry);

	UE_LOG(LogMessages, Log, TEXT("[Sys] %s"), *Message);
}


/**
* Stub. This is just a possible example.
*/
void UUR_MessageHistory::OnReceiveDeathMessage(APlayerState* Killer, APlayerState* Victim, TSubclassOf<UDamageType> DmgType)
{
	FMessageHistoryEntry Entry;
	Entry.Time = FDateTime::Now();
	Entry.Type = MessageType::Death;

	Entry.Parts = { TEXT("%1 killed %2"), TEXT("Somebody"), TEXT("Somebody") };
	Entry.Colors = { FColor::White, FColor::White };

	if (!Killer || Killer == Victim)
	{
		// Suicide
		// DmgType->GetSuicideString()
		Entry.Parts[0] = FString::Printf(TEXT("%2 suicided with %s"), DmgType ? *DmgType->GetName() : TEXT("???"));
	}
	else
	{
		// Kill
		// DmgType->GetKillString()
		Entry.Parts[0] = FString::Printf(TEXT("%1 killed %2 with %s"), DmgType ? *DmgType->GetName() : TEXT("???"));
	}

	if (Killer)
	{
		Entry.Parts[1] = Killer->GetPlayerName();
		Entry.Colors[0] = UUR_FunctionLibrary::GetPlayerDisplayTextColor(Killer);
	}

	if (Victim)
	{
		Entry.Parts[2] = Victim->GetPlayerName();
		Entry.Colors[1] = UUR_FunctionLibrary::GetPlayerDisplayTextColor(Victim);
	}

	Append(Entry);

	// LogDeath?
	UE_LOG(LogMessages, Log, TEXT("%s"), *Entry.Parts[0].Replace(TEXT("%1"), *Entry.Parts[1]).Replace(TEXT("%2"), *Entry.Parts[2]));
}
