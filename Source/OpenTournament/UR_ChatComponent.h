// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UR_ChatComponent.generated.h"


/**
* Chat message channels.
* TODO: move to global types?
*/
UENUM(BlueprintType)
enum class EChatChannel : uint8
{
	None,		/** not sure what this could be used for, maybe messages for console only */
	System,		/** system messages - we could use this for PC->ClientMessage */
	Say,		/** messages for everyone in the game */
	Team,		/** messages for a specific team only */
	Spec,		/** messages for spectators only */
};

/**
* Event dispatcher for receiving a message.
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FReceiveChatMessageSignature, const FString&, SenderName, const FString&, Message, EChatChannel, Channel, APlayerState*, SenderPS);

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
	* Cache to owner PlayerController if there is one.
	* May be NULL for non-player entities.
	*/
	UPROPERTY(BlueprintReadOnly)
	class AController* OwnerController;

	/**
	* Sender name to use when we cannot resolve to a PlayerState object.
	*/
	UPROPERTY(EditAnywhere)
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
	* Max message length
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
	virtual void Send(const FString& Message, EChatChannel Channel);

	/**
	* Called by client and then server to validate the chat message before broadcast.
	* Returns the validated message (may be modified).
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category = "Chat")
	FString Validate(const FString& Message, EChatChannel Channel);

	/**
	* Server RPC to send chat message.
	*/
	UFUNCTION(Server, Reliable, WithValidation)
	virtual void ServerSend(const FString& Message, EChatChannel Channel);

	/**
	* Broadcast a chat message to the appropriate receivers.
	*/
	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "Chat")
	virtual void Broadcast(const FString& Message, EChatChannel Channel);

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
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Chat")
	bool ShouldReceive(EChatChannel Channel, UUR_ChatComponent* Sender);

	/**
	* Receive a chat message (server)
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Chat")
	void Receive(UUR_ChatComponent* Sender, const FString& Message, EChatChannel Channel);

	/**
	* Client RPC to receive chat message.
	* SenderName is the server-resolved name of message author.
	* SenderPS is the player state of author if there is one. Might be null for non-player senders.
	*/
	UFUNCTION(Client, Reliable)
	virtual void ClientReceive(const FString& SenderName, const FString& Message, EChatChannel Channel, APlayerState* SenderPS);

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
};
