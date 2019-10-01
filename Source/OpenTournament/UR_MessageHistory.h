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

DECLARE_LOG_CATEGORY_EXTERN(LogChat, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogMessages, Log, All);


/**
* Direct model for our MessageHistory/HistoryMessage UMG widget.
*
* - A field for Timestamp, formatted in umg/bp.
* - A field for the Type, filterable and localized in umg/bp.
* - A field for the message, formatted in c++ and displayed via RichTextBlock.
*
* This is the simplest way to do it, because we have lots of different kind of messages, formatted in a different way.
*
* Still, I would like to format in blueprint because it makes more sense.
* The UMG interface displays the things, and should be the one that formats as well.
*
* Two examples of things bothering me right now :
*
* 1) For chat message we cannot get rid of the colon between player and message in BP,
* or change whitespaces around it. That should be the designer job, not c++, so it feels wrong.
*
* 2) I cannot colorize the Type column appropriately for TeamChat,
* because I don't have the team/color info, because it's already formatted into FormattedText.
*
* I can see one possible solution.
* Instead of fully formatting in c++, we can instead prepare a TArray/TMap with parts.
* For example, a chat message could be represented by ["PlayerName", "#f00", "Some message"],
* and then BP would easily reconstruct the RichText line using Type + the parts.
*
* The above solution seems good because it is very modular. We can still have a part that acts as a template.
* For example for death messages we could have ["%1 sniped %2", "player1", "#f00", "player2", "#00f"]
* (The template in this case should be the resolved/localized death string pulled from DamageType)
* And BP can reconstruct message easily, choose whether to use colors or not (designer decision!), etc.
*
* Would probably be better with an array/map of strings and an array of colors.
*
* Not important but small problem = we kind of fuck up the logging part which was easy now.
*/
USTRUCT(BlueprintType)
struct FMessageHistoryEntry
{
	GENERATED_BODY()

	/**
	* Time stamp of receiving.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FDateTime Time;

	/**
	* Type of the message. Used for filtering.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName Type;

	/**
	* Formatted line of text with rich-text markers, ready to be displayed in RichTextBlock.
	* - RichTextBlock requires the RichTextDecorator_CustomStyle decorator class.
	* - You can use the StripDecorators utility to use as plain text.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString FormattedText;
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
	* Persistent global history of messages.
	*/
	UPROPERTY(BlueprintReadOnly)
	TArray<FMessageHistoryEntry> History;

	/**
	* Event dispatcher upon adding an entry to history.
	*/
	UPROPERTY(BlueprintAssignable)
	FNewMessageHistoryEntrySignature OnNewMessageHistoryEntry;

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
	* Listen to system messages, and add them to history.
	*/
	UFUNCTION()
	virtual void OnReceiveSystemMessage(const FString& Message);

	/**
	* Listen to death messages, and add them to history.
	* This is a stub.
	*/
	UFUNCTION()
	virtual void OnReceiveDeathMessage(APlayerState* Killer, APlayerState* Victim, TSubclassOf<UDamageType> DmgType);

	/**
	* Utility.
	*/
	virtual FString FormattedPlayerName(APlayerState* PS);
};
