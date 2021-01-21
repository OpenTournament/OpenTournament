// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_MessageHistory.h"

#include "Engine/World.h"
#include "GameFramework/DamageType.h"

#include "UR_FunctionLibrary.h"
#include "UR_ChatComponent.h"
#include "UR_PlayerState.h"
#include "UR_GameState.h"
#include "UR_PlayerController.h"
#include "UR_Pickup.h"

#include "Slate/UR_RichTextDecorator_CustomStyle.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

DEFINE_LOG_CATEGORY(LogChat);
DEFINE_LOG_CATEGORY(LogMessages);
DEFINE_LOG_CATEGORY(LogFrags);
DEFINE_LOG_CATEGORY(LogPickups);

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_MessageHistory::UUR_MessageHistory()
{
    History.Reserve(MESSAGES_HISTORY_MAX);
}

void UUR_MessageHistory::InitWithPlayer(AUR_PlayerController* PC)
{
    PC->ChatComponent->OnReceiveChatMessage.AddUniqueDynamic(this, &UUR_MessageHistory::OnReceiveChatMessage);

    PC->OnReceiveSystemMessage.AddUniqueDynamic(this, &UUR_MessageHistory::OnReceiveSystemMessage);

    GetWorld()->GameStateSetEvent.AddUObject(this, &UUR_MessageHistory::OnGameStateCreated);
    OnGameStateCreated(GetWorld()->GetGameState());
}

void UUR_MessageHistory::OnGameStateCreated(AGameStateBase* GS)
{
    if (AUR_GameState* URGS = Cast<AUR_GameState>(GS))
    {
        URGS->OnMatchSubStateChanged.AddUniqueDynamic(this, &UUR_MessageHistory::OnMatchSubStateChanged);
        URGS->FragEvent.AddUniqueDynamic(this, &UUR_MessageHistory::OnFrag);
        URGS->PickupEvent.AddUniqueDynamic(this, &UUR_MessageHistory::OnPickup);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

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

void UUR_MessageHistory::OnMatchSubStateChanged(AUR_GameState* GS)
{
    FMessageHistoryEntry Entry;
    Entry.Time = FDateTime::Now();
    Entry.Type = MessageType::Match;
    Entry.Parts = { GS->GetMatchSubState().ToString() };
    Entry.Colors.Empty();
    Append(Entry);

    //NOTE: no logging here, there is already Match/Sub/State logging in GameState class.
}

void UUR_MessageHistory::OnFrag(AUR_PlayerState* Victim, AUR_PlayerState* Killer, TSubclassOf<UDamageType> DmgType, const TArray<FName>& Extras)
{
    if (!IsValid(Victim) && !IsValid(Killer))
    {
        return;
    }

    FMessageHistoryEntry Entry;
    Entry.Time = FDateTime::Now();
    Entry.Type = MessageType::Death;

    Entry.Parts = { TEXT("$1 killed $2"), TEXT("Somebody"), TEXT("Somebody") };
    Entry.Colors = { FColor::White, FColor::White };

    if (!Killer || Killer == Victim)
    {
        // Suicide
        // DmgType->GetSuicideString()
        Entry.Parts[0] = FString::Printf(TEXT("$2 suicided with %s"), DmgType ? *DmgType->GetName() : TEXT("???"));
    }
    else
    {
        // Kill
        // DmgType->GetKillString()
        Entry.Parts[0] = FString::Printf(TEXT("$1 killed $2 with %s"), DmgType ? *DmgType->GetName() : TEXT("???"));
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

    for (const FName& Extra : Extras)
    {
        Entry.Parts.Add(Extra.ToString());
    }

    Append(Entry);

    if (Extras.Num() > 0)
    {
        UE_LOG(LogFrags, Log, TEXT("%s [%s]"), *Entry.Parts[0].Replace(TEXT("$1"), *Entry.Parts[1]).Replace(TEXT("$2"), *Entry.Parts[2]), *FString::JoinBy(Extras, TEXT(","), [](const FName& Name) { return Name.ToString(); }));
    }
    else
    {
        UE_LOG(LogFrags, Log, TEXT("%s"), *Entry.Parts[0].Replace(TEXT("$1"), *(Entry.Parts[1])).Replace(TEXT("$2"), *(Entry.Parts[2])));
    }
}

void UUR_MessageHistory::OnPickup(AUR_Pickup* Pickup, AUR_PlayerState* Recipient)
{
    if (!IsValid(Pickup) && !IsValid(Recipient))
    {
        return;
    }

    FMessageHistoryEntry Entry;
    Entry.Time = FDateTime::Now();
    Entry.Type = MessageType::Pickup;

    Entry.Parts = { TEXT("$1 picked up $2"), TEXT("Somebody"), TEXT("Something") };
    Entry.Colors = { FColor::White };

    if (Recipient)
    {
        Entry.Parts[1] = Recipient->GetPlayerName();
        Entry.Colors[0] = UUR_FunctionLibrary::GetPlayerDisplayTextColor(Recipient);
    }

    if (Pickup)
    {
        Entry.Parts[2] = Pickup->DisplayName;
    }

    Append(Entry);

    UE_LOG(LogPickups, Log, TEXT("%s"), *Entry.Parts[0].Replace(TEXT("$1"), *Entry.Parts[1]).Replace(TEXT("$2"), *Entry.Parts[2]));
}
