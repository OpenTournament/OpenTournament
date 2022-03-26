// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "UR_MessageHistory.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AUR_PlayerController;
class AUR_PlayerState;
class UDamageType;

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace MessageType
{
	// Chat
	static const FName GlobalChat = FName(TEXT("MSG_GlobalChat"));
	static const FName TeamChat = FName(TEXT("MSG_TeamChat"));
	static const FName SpecChat = FName(TEXT("MSG_SpecChat"));

	// System messages
	static const FName System = FName(TEXT("MSG_System"));

    // Match state
    static const FName Match = FName(TEXT("MSG_Match"));

    // Death/frags
    static const FName Death = FName(TEXT("MSG_Death"));

    // Pickups
    static const FName Pickup = FName(TEXT("MSG_Pickup"));

	// Some other examples - not implemented yet
	static const FName Objectives = FName(TEXT("MSG_Objective"));
	static const FName BotChat = FName(TEXT("MSG_BotChat"));
	static const FName BotTaunt = FName(TEXT("MSG_BotTaunt"));
};

/////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FMessageHistoryFilters
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bGlobal;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bTeam;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSpec;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSystem;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bMatch;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bFrags;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPickups;

    FMessageHistoryFilters() : bGlobal(false), bTeam(false), bSpec(false), bSystem(false), bMatch(false), bFrags(false), bPickups(false) {}
};

/////////////////////////////////////////////////////////////////////////////////////////////////

DECLARE_LOG_CATEGORY_EXTERN(LogChat, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogMessages, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogFrags, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogPickups, Log, All);

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
* Data structure MessageHistory/HistoryMessage UMG widget.
*
* - A timestamp, to be formatted in umg/bp.
* - A type field, to be filterable and localized in umg/bp.
* - An array of text parts, which are to be put together in umg/bp.
* - An array of colors, which are to be used (or not) by umg/bp.
*
* Examples
*
* A chat message is represented by the following :
* - Type = MSG_Chat
* - Parts = [ "PlayerName", "The message." ]
* - Colors = [ #PlayerTeamColor, #ChannelTeamColor ]
*
* A death message is represented by the following :
* - Type = MSG_Death
* - Parts = [ "KillerName", "VictimName", "$1 killed $2 with shock." ]
* - Colors = [ #KillerTeamColor, #VictimTeamColor ]
*
* This gives the designer side a lot of flexibility to display history in the desired way,
* without being stuck with C++ pre-formatted strings.
* 
* The only downside is that internal logging might not match exactly what is displayed on screen,
* since logging is done in C++ while formatting is done in umg/bp.
* But it matters little as the logs should not be visible to end user.
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
	* Instead of one fully formatted rich string done in c++,
	* we simply prepare text parts and let BP/UMG reconstruct the final line.
	*
	* This gives the designer (UMG) more liberty to format/design the line,
	* in whatever way he sees fit.
	*
	* And it does so without holding object references or being restricted
	* to a fixed set of variables, like we had in first iteration.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FText> Parts;

	/**
	* Similarly we put colors of interest (eg: players colors...) here in an array,
	* so they can be used for formatting (or not, designer decision).
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FColor> Colors;

    FMessageHistoryEntry() : Time(0) {}
};

/////////////////////////////////////////////////////////////////////////////////////////////////

#define MESSAGES_HISTORY_MAX 1000

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNewMessageHistoryEntrySignature, const FMessageHistoryEntry&, Entry);

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * 
 */
UCLASS(BlueprintType, Config = UI)
class OPENTOURNAMENT_API UUR_MessageHistory : public UObject
{
	GENERATED_BODY()

	UUR_MessageHistory();

    virtual void OnGameStateCreated(class AGameStateBase* GS);

    UFUNCTION()
    virtual void OnViewTargetChanged(class AUR_BasePlayerController* PC, AActor* NewVT, AActor* OldVT);

public:

    virtual void InitWithPlayer(AUR_PlayerController* PC);

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
	* Saved filters.
	*/
	UPROPERTY(Config, BlueprintReadWrite)
	FMessageHistoryFilters SavedFilters;

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

    UFUNCTION()
    virtual void OnMatchStateTagChanged(AUR_GameState* GS);

    UFUNCTION()
    virtual void OnFrag(AUR_PlayerState* Victim, AUR_PlayerState* Killer, TSubclassOf<UDamageType> DmgType, const FGameplayTagContainer& Tags);

    UFUNCTION()
    virtual void OnGlobalPickup(TSubclassOf<AUR_Pickup> PickupClass, AUR_PlayerState* Recipient);

    UFUNCTION()
    virtual void OnCharacterPickup(AUR_Pickup* Pickup);

	/**
	* Expose SaveConfig to blueprints so widget can manipulate filters.
	*/
	UFUNCTION(BlueprintCallable, Meta = (DisplayName = "Save Config"))
	virtual void K2_SaveConfig() { SaveConfig(); }
};
