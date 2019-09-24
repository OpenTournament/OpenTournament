// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_MessageHistory.h"

#include "UR_FunctionLibrary.h"
#include "UR_ChatComponent.h"
#include "UR_PlayerState.h"


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
	UUR_MessageHistory::MakeChatHistoryEntry(SenderName, Message, TeamIndex, SenderPS, Entry);
	Append(Entry);
}

void UUR_MessageHistory::MakeChatHistoryEntry(const FString& SenderName, const FString& Message, int32 TeamIndex, APlayerState* SenderPS, FMessageHistoryEntry& OutEntry)
{
	FColor MessageColor;
	if (TeamIndex >= 0)
	{
		// TODO: team color
		MessageColor = FColorList::Red;
	}
	else if (TeamIndex == CHAT_INDEX_SPEC)
		MessageColor = UUR_FunctionLibrary::GetSpectatorDisplayTextColor();
	else
		MessageColor = FColor::White;

	OutEntry.Time = FDateTime::Now();
	OutEntry.Type = TeamIndex >= 0 ? MessageType::TeamChat : (TeamIndex == CHAT_INDEX_SPEC ? MessageType::SpecChat : MessageType::GlobalChat);
	OutEntry.Message = Message;
	OutEntry.Color = MessageColor;
	OutEntry.Player1 = { SenderName, UUR_FunctionLibrary::GetPlayerDisplayTextColor(SenderPS) };
	OutEntry.OptionalObject = nullptr;
}


void UUR_MessageHistory::OnReceiveSystemMessage(const FString& Message)
{
	FMessageHistoryEntry Entry;
	UUR_MessageHistory::MakeSystemHistoryEntry(Message, Entry);
	Append(Entry);
}

void UUR_MessageHistory::MakeSystemHistoryEntry(const FString& Message, FMessageHistoryEntry& OutEntry)
{
	OutEntry.Time = FDateTime::Now();
	OutEntry.Type = MessageType::System;
	OutEntry.Message = Message;
	OutEntry.Color = FColorList::LightGrey;
	OutEntry.OptionalObject = nullptr;
}


/**
* I am starting to think that storing history as an array of FString would be alot simpler,
* using RichTextBlock to have encoded colors and things.
*
* Here we are going to be stuck if we want to have varied or complex death messages, like :
* - A killed B with sniper
* - B got rekt by A
*
* This is very difficult to handle in UMG if we use different text blocks for elements (player, text, player, text...)
*
* Besides, keeping a reference to OptionalObject will probably freak GC out.
*/
void UUR_MessageHistory::OnReceiveDeathMessage(APlayerState* Killer, APlayerState* Victim, TSubclassOf<UDamageType> DmgType)
{
	FMessageHistoryEntry Entry;
	Entry.Time = FDateTime::Now();
	Entry.Type = MessageType::Death;
	Entry.Player1 = { Killer->GetPlayerName(), UUR_FunctionLibrary::GetPlayerDisplayTextColor(Killer) };
	Entry.Player2 = { Victim->GetPlayerName(), UUR_FunctionLibrary::GetPlayerDisplayTextColor(Victim) };
	Entry.OptionalObject = DmgType;

	Append(Entry);
}
