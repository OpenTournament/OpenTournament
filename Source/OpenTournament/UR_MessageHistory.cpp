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

	// Log a plain text version
	FString PlainText = UUR_FunctionLibrary::UnescapeRichText(UUR_FunctionLibrary::StripRichTextDecorators(Entry.FormattedText));
	if (Entry.Type.ToString().EndsWith(TEXT("Chat")) || Entry.Type == MessageType::BotTaunt)
	{
		UE_LOG(LogChat, Log, TEXT("%s"), *PlainText);
	}
	else
	{
		UE_LOG(LogMessages, Log, TEXT("%s"), *PlainText);
	}

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

	FString FormattedAuthor(TEXT(""));
	FString FormattedMessage(Message);

	if (SenderName.Len() > 0)
	{
		FormattedAuthor = UUR_RichTextDecorator_CustomStyle::DecorateRichText(SenderName, true, UUR_FunctionLibrary::GetPlayerDisplayTextColor(SenderPS));
		FormattedMessage.InsertAt(0, TEXT(" : "));
	}

	FormattedMessage = UUR_RichTextDecorator_CustomStyle::DecorateRichText(FormattedMessage, true, UUR_ChatComponent::GetChatMessageColor(this, TeamIndex));

	Entry.FormattedText = FString::Printf(TEXT("%s%s"), *FormattedAuthor, *FormattedMessage);

	Append(Entry);
}


void UUR_MessageHistory::OnReceiveSystemMessage(const FString& Message)
{
	FMessageHistoryEntry Entry;
	Entry.Time = FDateTime::Now();
	Entry.Type = MessageType::System;
	Entry.FormattedText = Message;
	Append(Entry);
}


/**
* Stub. This is just a possible example.
*/
void UUR_MessageHistory::OnReceiveDeathMessage(APlayerState* Killer, APlayerState* Victim, TSubclassOf<UDamageType> DmgType)
{
	FMessageHistoryEntry Entry;
	Entry.Time = FDateTime::Now();
	Entry.Type = MessageType::Death;

	// will probably need something along the lines of,
	// DmgType->GetKillString().Replace(TEXT("%1"), Killer).Replace(TEXT("%2"), Victim)

	if (!Killer || Killer == Victim)
	{
		Entry.FormattedText = FString::Printf(TEXT("%s suicided with %s"),
			*FormattedPlayerName(Killer),
			DmgType ? *DmgType->GetName() : TEXT("???")
		);
	}
	else
	{
		Entry.FormattedText = FString::Printf(TEXT("%s killed %s with %s"),
			*FormattedPlayerName(Killer),
			*FormattedPlayerName(Victim),
			DmgType ? *DmgType->GetName() : TEXT("???")
		);
	}

	Append(Entry);
}


FString UUR_MessageHistory::FormattedPlayerName(APlayerState* PS)
{
	if (PS)
		return UUR_RichTextDecorator_CustomStyle::DecorateRichText(PS->GetPlayerName(), true, UUR_FunctionLibrary::GetPlayerDisplayTextColor(PS));

	return TEXT("Somebody");
}
