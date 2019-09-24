// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UR_MessageHistory.generated.h"


namespace MessageType
{
	// Chat
	const FName GlobalChat = FName(TEXT("MSG_GlobalChat"));
	const FName TeamChat = FName(TEXT("MSG_TeamChat"));
	const FName SpecChat = FName(TEXT("MSG_SpecChat"));

	// System messages
	const FName System = FName(TEXT("MSG_System"));

	// Some other examples - not implemented yet
	const FName Death = FName(TEXT("MSG_Death"));
	const FName Pickup = FName(TEXT("MSG_Pickup"));
	const FName Objectives = FName(TEXT("MSG_Objective"));
	const FName BotChat = FName(TEXT("MSG_BotChat"));
	const FName BotTaunt = FName(TEXT("MSG_BotTaunt"));
};


/**
* For storing the equivalent of a PlayerState in persistent history.
* I believe all we need (for now) is player name and his color.
*/
USTRUCT(BlueprintType)
struct FMessageHistoryPlayer
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString Name;

	UPROPERTY(BlueprintReadOnly)
	FColor Color;
};


/**
* Messages history now needs to be able to store all kind of message events.
*
* Chat messages are one type of message.
* They come with an author and a text, and a color if team channel.
*
* Most other events should come through LocalizedMessage.
* LocalizedMessage provides parameters for 2 PlayerState and 1 OptionalObject.
* For example, death messages typically use them for Killer, Victim, and DamageType.
* In order to have accurate reconstruction we need to provide storage for those as well.
* (because we still want colorized player names in death messages history)
* (and maybe weapon icons if we decide to use weapon icons death messages)
*/
USTRUCT(BlueprintType)
struct FMessageHistoryEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FDateTime Time;

	UPROPERTY(BlueprintReadOnly)
	FName Type;

	UPROPERTY(BlueprintReadOnly)
	FString Message;

	/**
	* Color of the message if applicable.
	* - For chat messages, should be the appropriate color (team,spec,global).
	*/
	UPROPERTY(BlueprintReadOnly)
	FColor Color;

	/**
	* Generic player info 1.
	* - For chat messages, should be the sender.
	* - For death messages, should be the killer.
	*/
	UPROPERTY(BlueprintReadOnly)
	FMessageHistoryPlayer Player1;

	/**
	* Generic player info 2.
	* - For death messages, should be the victim.
	*/
	UPROPERTY(BlueprintReadOnly)
	FMessageHistoryPlayer Player2;

	/**
	* Generic object reference.
	* - For death messages, should be DamageType.
	*
	* WARNING: Maybe should use UClass instead of UObject ? I fear for garbage collection.
	*/
	UPROPERTY(BlueprintReadOnly)
	UObject* OptionalObject;
};


#define MESSAGES_HISTORY_MAX 1000


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNewMessageHistoryEntrySignature, const FMessageHistoryEntry&, Entry);


/**
 * 
 */
UCLASS(BlueprintType)
class OPENTOURNAMENT_API UUR_MessageHistory : public UObject
{
	GENERATED_BODY()

	UUR_MessageHistory(const FObjectInitializer& ObjectInitializer);

public:

	/**
	* Persistent history of messages.
	*
	* NOTE: As we are moving on to a more general MessagesHistory,
	* there is some design choice to make here.
	*
	* Chat messages for example need an Author (name + team/color),
	* message text, message team/color.
	*
	* Death messages need two players (name + team/color),
	* and a weapon/damagetype reference.
	*
	* Other type of messages will have their own set of data as well.
	* Besides, history needs to be bigger to store a good amount of all types
	* and have them filterable.
	*
	* We can either make one "big" struct to cover all possible types,
	* and use static pre-allocated array of 10000(?) elements.
	* This is overall simpler but comes at a bigger memory cost.
	*
	* Other possibility is an array of pointers to different type of structs,
	* unfortunately that is not supported in Blueprints.
	* That means we have to provide functions to iterate or convert the array on demand.
	*
	* Yet another possibility : we could simply store an array of FString,
	* with colors encoded inside the text. And use some kind of RichText to display them in UMG.
	*/
	UPROPERTY(BlueprintReadOnly)
	TArray<FMessageHistoryEntry> History;


	/**
	* Event dispatcher upon adding an entry to history.
	*/
	UPROPERTY(BlueprintAssignable)
	FNewMessageHistoryEntrySignature OnNewMessageHistoryEntry;


	/**
	* IF we go the pointers route, here is an idea of exposer/converter that could do the job.
	*
	* We still need a "big struct" that can encompass all types, but its memory footprint would be temporary,
	* pre-filtered, and only as big as passed in MaxSize (and not the full history).
	*
	* Assumes we have a base struct FMessageHistoryEntry{ FDateTime Time, FName Type }
	* And subclassed structs such as FChatHistoryEntry and FDeathHistoryEntry
	* And the shared struct FSharedHistoryEntry for blueprint usage.
	*/
	/*
	TArray<TSharedPtr<FMessageHistoryEntry>> MessagesHistory;

	UFUNCTION(BlueprintCallable)
	void LoadMessagesHistory(const uint32 MaxSize, const TSet<FName>& Filters, TArray<FChatHistoryEntry>& OutHistory)
	{
		OutHistory.Empty(MaxSize);
		for (const TSharedPtr<FMessageHistoryEntry>& Entry : MessagesHistory)
		{
			if (Filters.Contains(Entry->Type))
			{
				switch (Entry->Type)
				{
					case MessageType::GlobalChat :
					case MessageType::TeamChat :
					case MessageType::SpecChat :
						OutHistory.Emplace( FSharedHistoryEntry::MakeFromChat( StaticCastSharedPtr<FChatHistoryEntry>(Entry) );
						break;

					case MessageType::Death :
						OutHistory.Emplace( FSharedHistoryEntry::MakeFromDeath( StaticCastSharedPtr<FDeathHistoryEntry>(Entry) );
						break;
				}
			}
		}
	}
	*/


	/**
	* Add an entry to the history, and trigger dispatcher.
	*/
	UFUNCTION(BlueprintCallable, Category = "History")
	virtual void Append(const FMessageHistoryEntry& Entry);


	/**
	* Listen to chat messages, and add them to history.
	*/
	UFUNCTION()
	virtual void OnReceiveChatMessage(const FString& SenderName, const FString& Message, int32 TeamIndex, APlayerState* SenderPS);

	/**
	* Make a FMessageHistoryEntry for a broadcasted chat message.
	*/
	UFUNCTION(BlueprintCallable, Category = "History|Chat")
	static void MakeChatHistoryEntry(const FString& SenderName, const FString& Message, int32 TeamIndex, APlayerState* SenderPS, FMessageHistoryEntry& OutEntry);


	/**
	* Listen to system messages, and add them to history.
	*/
	UFUNCTION()
	virtual void OnReceiveSystemMessage(const FString& Message);

	/**
	* Make a FMessageHistoryEntry for a system message.
	*/
	UFUNCTION(BlueprintCallable, Category = "History")
	static void MakeSystemHistoryEntry(const FString& Message, FMessageHistoryEntry& OutEntry);


	/**
	* Listen to death messages, and add them to history.
	* This is a stub.
	*/
	UFUNCTION()
	virtual void OnReceiveDeathMessage(APlayerState* Killer, APlayerState* Victim, TSubclassOf<UDamageType> DmgType);
};
