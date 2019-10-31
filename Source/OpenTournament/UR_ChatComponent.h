// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UR_ChatComponent.generated.h"


/**
* Some notes on TeamIndex and why I got rid of the Channel ENUM :
*
* I did not like channel TEAM because it was not clear who were the recipients of the message.
* You had to resolve the sender team first.
* This complicates things in case we want for example to send a message to a team channel, without being in the team.
*
* Same problem when receiving a team message.
* You could assume a team message is obviously the team of the sender, or the team of the recipient.
* But what if you want server admin to be able to listen to other team chat? Or send team chat to other team?
* Or something like a chat logger, who has no team but needs to listen to all.
*
* So I prefer specifying TeamIndex explicitly when calling Broadcast(),
* and receiving TeamIndex explicitly in Receive(), to make all of these use-cases possible.
*
* Now second thing is, I don't like having both a Channel variable, and a TeamIndex variable being relevant only when Channel=TEAM.
* Since Team indexes are only positive, we can use negative indexes for Global and Spec cases.
* Then we can get rid of the Channel variable which is redundant.
*
* By convention we'll say this :
* -1 = Global channel
* -2 = Spectators channel
*
* Lastly, "system" messages do not belong in here. Sure they will be shown in chat box in the end but,
* similarly to other events (see ChatHistory.h), they dont need the ChatComponent mechanisms.
* System messages will either come through PC->ClientMessage, or the LocalizedMessage route like all other events.
* And, like other events, they should have their own dispatcher (which the chat box will listen to).
*
* At first I thought about routing all events through the OnChatMessageReceived dispatcher, but quickly realized it's wrong design.
* Here's how I rather see things working :
* - The ChatComponent will do its own thing, and trigger its own dispatcher.
* - The death message will go its own LocalizedMessage route and trigger its own dispatcher (in PC probably).
* History listens to both, and stores both, assigning each a MessageType that will be filterable.
*/

#define CHAT_INDEX_GLOBAL -1
#define CHAT_INDEX_SPEC -2


/**
* Event dispatcher for receiving a chat message.
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FReceiveChatMessageSignature, const FString&, SenderName, const FString&, Message, int32, TeamIndex, APlayerState*, SenderPS);


/**
* Component to make an entity chat-aware and chat-capable.
*
* Standard usage is for players, but it is designed to be useable by any Actor entity.
*
* For players/bots it should be owned by the Controller.
* We don't want to replicate to anybody else than the player owner.
*
* Standard implementation flows as following :
* - client types in message and press enter, calling Send() on client
* - message is validated through Validate(), then sent to server via ServerSend()
* - ServerSend() calls Send() (on server this time) which validates again, and forwards to Broadcast()
* - Broadcast() iterates all registered chat components and calls Receive() when component ShouldReceive()
* - Receive() then replicates through client RPC ClientReceive()
*
* For non-player entities you should set Replicates=false, specify FallbackOwnerName,
* and adjust AntiSpamDelay sensibly.
*
* For example, a webadmin system could use it to interface chat,
* without hacking in a fake spectator player/PRI like previous UT titles.
*/
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class OPENTOURNAMENT_API UUR_ChatComponent : public UActorComponent
{
	GENERATED_BODY()

	UUR_ChatComponent();

public:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	* Cache to owner Controller if there is one.
	* May be NULL for non-player entities.
	*/
	UPROPERTY(BlueprintReadOnly)
	class AController* OwnerController;

	/**
	* Sender name to use when we cannot resolve to a PlayerState object.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString FallbackOwnerName;

	/**
	* Minimum delay in seconds between two messages from this sender.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AntiSpamDelay;

	/**
	* Timestamp at which last message was sent by this sender.
	* Updated by function Send() after checking with Validate(), so it can be bypassed by calling Broadcast() directly.
	*/
	UPROPERTY(BlueprintReadOnly)
	float LastSendTime;

	/**
	* Max message length. Longer messages will be cropped.
	*/
	UPROPERTY(BlueprintReadOnly)
	int32 MaxMessageLength;

	/**
	* Event dispatcher for receiving a message.
	*/
	UPROPERTY(BlueprintAssignable)
	FReceiveChatMessageSignature OnReceiveChatMessage;

	/**
	* Entry point to (try to) send a message.
	* Should be called from client exec commands.
	*/
	UFUNCTION(BlueprintCallable, Category = "Chat")
	virtual void Send(const FString& Message, bool bTeamMessage);

	/**
	* Called by client and then server to validate the chat message before broadcast.
	* Returns the validated message (may be modified).
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category = "Chat")
	FString Validate(const FString& Message, bool bTeamMessage);

	/**
	* Server RPC to send chat message.
	*/
	UFUNCTION(Server, Reliable, WithValidation)
	virtual void ServerSend(const FString& Message, bool bTeamMessage);

	/**
	* Broadcast a chat message to the appropriate receivers.
	* Use TeamIndex -1 for global chat.
	* Use TeamIndex -2 for spectator channel.
	*/
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Chat")
	virtual void Broadcast(const FString& Message, int32 TeamIndex = -1);

	/**
	* Resolve this component Team Index.
	* NOTE: if we end up using some sort of "Team Interface" like UT4 does, this will have to be adjusted.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Chat")
	int32 GetTeamIndex();

	/**
	* Resolve this component's owner name.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category = "Chat")
	FString GetOwnerName();

	/**
	* Try to resolve this component's owner PlayerState.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category = "Chat")
	APlayerState* GetPlayerState();

	/**
	* Check whether this component should receive a broadcasted message.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintAuthorityOnly, Category = "Chat")
	bool ShouldReceive(UUR_ChatComponent* Sender, int32 TeamIndex);

	/**
	* Receive a chat message (server).
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintAuthorityOnly, Category = "Chat")
	void Receive(UUR_ChatComponent* Sender, const FString& Message, int32 TeamIndex);

	/**
	* Client RPC to receive chat message.
	* SenderName is the server-resolved name of message author.
	* SenderPS is the player state of author if there is one. Might be null for non-player senders.
	*/
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Chat")
	virtual void ClientReceive(const FString& SenderName, const FString& Message, int32 TeamIndex, APlayerState* SenderPS);

	/**
	* Process parameters in chat messages.
	* %t : match clock
	* %h : my health
	* %a : my armor
	* %w : my weapon
	* %l : my location
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category = "Chat")
	FString ProcessChatParameters(const FString& Original);

	/**
	* Micro helper to translate TeamIndex to message display color.
	* This is to avoid code duplication between c++ history and blueprint chatbox.
	*/
	UFUNCTION(BlueprintPure, Category = "Chat", Meta = (WorldContext = "WorldContextObject"))
	static FColor GetChatMessageColor(UObject* WorldContextObject, int32 TeamIndex);
};
