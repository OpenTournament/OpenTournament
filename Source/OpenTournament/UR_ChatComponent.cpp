// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_ChatComponent.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "UR_GameModeBase.h"

#include "UR_GameState.h"
#include "UR_Character.h"
#include "UR_HealthComponent.h"

UUR_ChatComponent::UUR_ChatComponent()
	: OwnerPC(nullptr)
	, FallbackOwnerName(TEXT("THING"))
	, AntiSpamDelay(1.f)
	, LastSendTime(0)
	, MaxMessageLength(150)
	
{
	bReplicates = true;
}

void UUR_ChatComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerPC = Cast<APlayerController>(GetOwner());

	AUR_GameModeBase* GM = GetWorld()->GetAuthGameMode<AUR_GameModeBase>();
	if (GM)
		GM->RegisterChatComponent(this);
}

void UUR_ChatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	AUR_GameModeBase* GM = GetWorld()->GetAuthGameMode<AUR_GameModeBase>();
	if (GM)
		GM->UnregisterChatComponent(this);
}

void UUR_ChatComponent::Send(const FString& Message, EChatChannel Channel)
{
	FString ValidatedMessage = Validate(ProcessChatParameters(Message), Channel);
	if (!ValidatedMessage.IsEmpty())
	{
		LastSendTime = GetWorld()->RealTimeSeconds;

		if (GetOwnerRole() == ROLE_Authority)
			Broadcast(ValidatedMessage, Channel);
		else
			ServerSend(ValidatedMessage, Channel);
	}
}

bool UUR_ChatComponent::ServerSend_Validate(const FString& Message, EChatChannel Channel)
{
	return true;
}
void UUR_ChatComponent::ServerSend_Implementation(const FString& Message, EChatChannel Channel)
{
	Send(Message, Channel);
}

FString UUR_ChatComponent::Validate_Implementation(const FString& Message, EChatChannel Channel)
{
	// anti spam
	if (GetWorld()->RealTimeSeconds - LastSendTime < 1.f)
		return TEXT("");

	// trim
	FString ValidatedMessage = Message.TrimStartAndEnd();

	// crop message length
	if (ValidatedMessage.Len() > MaxMessageLength)
		ValidatedMessage = ValidatedMessage.Left(MaxMessageLength-3).Append(TEXT("..."));	// TEXT('…') not supported by font atm

	return ValidatedMessage;
}

void UUR_ChatComponent::Broadcast(const FString& Message, EChatChannel Channel)
{
	// Fixup the channel in some cases
	if (Channel == EChatChannel::Team)
	{
		if (OwnerPC && OwnerPC->PlayerState && OwnerPC->PlayerState->bOnlySpectator)
		{
			Channel = EChatChannel::Spec;
		}
		else
		{
			//TODO: if not a team game mode, set Channel = Say
		}
	}

	AUR_GameModeBase* GM = GetWorld()->GetAuthGameMode<AUR_GameModeBase>();
	if (GM)
	{
		for (auto Recipient : GM->ChatComponents)
		{
			if (Recipient->ShouldReceive(Channel, this))
			{
				Recipient->Receive(this, Message, Channel);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No gamemode available to broadcast message"));
	}
}

bool UUR_ChatComponent::ShouldReceive_Implementation(EChatChannel Channel, UUR_ChatComponent* Sender)
{
	switch (Channel)
	{
		case EChatChannel::System:
		case EChatChannel::Say:
			return true;

		case EChatChannel::Team:
		{
			if (!Sender)
				return false;

			//TODO: team implementation
			// if ( IsTeamGameMode() )
			//   return Sender.GetTeamIndex() == GetTeamIndex()
			// else
			return true;
		}

		case EChatChannel::Spec:
			return OwnerPC && OwnerPC->PlayerState && OwnerPC->PlayerState->bOnlySpectator;

		default:
			return false;
	}
}

void UUR_ChatComponent::Receive_Implementation(UUR_ChatComponent* Sender, const FString& Message, EChatChannel Channel)
{
	FString SenderName = Sender ? Sender->GetOwnerName() : TEXT("");
	APlayerState* SenderPS = Sender ? Sender->GetPlayerState() : nullptr;

	ClientReceive(SenderName, Message, Channel, SenderPS);

	OnReceiveChatMessage.Broadcast(SenderName, Message, Channel, SenderPS);
}

FString UUR_ChatComponent::GetOwnerName_Implementation()
{
	if ( OwnerPC && OwnerPC->PlayerState )
		return OwnerPC->PlayerState->GetPlayerName();

	return FallbackOwnerName;
}

APlayerState* UUR_ChatComponent::GetPlayerState_Implementation()
{
	return OwnerPC ? OwnerPC->PlayerState : nullptr;
}

void UUR_ChatComponent::ClientReceive_Implementation(const FString& SenderName, const FString& Message, EChatChannel Channel, APlayerState* SenderPS)
{
	FString Prefix(TEXT(""));
	switch (Channel)
	{
		case EChatChannel::System: Prefix = TEXT("[SYS] ");  break;
		case EChatChannel::Team:   Prefix = TEXT("[TEAM] "); break;
		case EChatChannel::Spec:   Prefix = TEXT("[SPEC] "); break;
	}

	if (!SenderName.IsEmpty())
		Prefix.Append(SenderName).Append(": ");

	UE_LOG(LogTemp, Log, TEXT("%s%s"), *Prefix, *Message);

	if ( GetOwnerRole() != ROLE_Authority )
		OnReceiveChatMessage.Broadcast(SenderName, Message, Channel, SenderPS);
}

FString UUR_ChatComponent::ProcessChatParameters_Implementation(const FString& Original)
{
	FString Result(Original);

	AUR_GameState* GameState = GetWorld()->GetGameState<AUR_GameState>();
	if (GameState)
	{
		float t = GameState->ElapsedTime;	//TODO: use real clock time
		Result.ReplaceInline(TEXT("%t"), *FString::Printf(TEXT("%02i:%02i"), FMath::FloorToInt(t / 60.f), FMath::RoundToInt(t) % 60));
	}

	if (OwnerPC)
	{
		AUR_Character* Char = Cast<AUR_Character>(OwnerPC->GetCharacter());
		if (Char)
		{
			Result.ReplaceInline(TEXT("%h"), *FString::Printf(TEXT("%i"), Char->HealthComponent->Health));
			Result.ReplaceInline(TEXT("%a"), *FString::Printf(TEXT("%i"), 0));	//TODO: armor
			Result.ReplaceInline(TEXT("%w"), TEXT("None"));	//TODO: weapon
			Result.ReplaceInline(TEXT("%l"), TEXT("Somewhere"));	//TODO: location
		}
	}

	return Result;
}