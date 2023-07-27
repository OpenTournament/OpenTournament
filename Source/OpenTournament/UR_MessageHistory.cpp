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
#include "UR_Character.h"

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
    PC->ChatComponent->OnReceiveChatMessage.AddUniqueDynamic(this, &ThisClass::OnReceiveChatMessage);

    PC->OnReceiveSystemMessage.AddUniqueDynamic(this, &ThisClass::OnReceiveSystemMessage);

    // Listen to gamestate events
    // NOTE: there should be only one gamestate at a time so no need to unbind previous
    GetWorld()->GameStateSetEvent.AddUObject(this, &ThisClass::OnGameStateCreated);
    OnGameStateCreated(GetWorld()->GetGameState());

    // Bind ViewTarget event so we can listen to events within the viewed character
    PC->OnViewTargetChanged.AddUniqueDynamic(this, &ThisClass::OnViewTargetChanged);
    OnViewTargetChanged(PC, PC->GetViewTarget(), nullptr);
}

void UUR_MessageHistory::OnGameStateCreated(AGameStateBase* GS)
{
    if (AUR_GameState* URGS = Cast<AUR_GameState>(GS))
    {
        URGS->OnMatchStateTagChanged.AddUniqueDynamic(this, &ThisClass::OnMatchStateTagChanged);
        URGS->FragEvent.AddUniqueDynamic(this, &ThisClass::OnFrag);
        URGS->PickupEvent.AddUniqueDynamic(this, &ThisClass::OnGlobalPickup);
    }
}

void UUR_MessageHistory::OnViewTargetChanged(class AUR_BasePlayerController* PC, AActor* NewVT, AActor* OldVT)
{
    if (auto OldChar = Cast<AUR_Character>(OldVT))
    {
        OldChar->PickupEvent.RemoveDynamic(this, &ThisClass::OnCharacterPickup);
    }
    if (auto NewChar = Cast<AUR_Character>(NewVT))
    {
        NewChar->PickupEvent.AddUniqueDynamic(this, &ThisClass::OnCharacterPickup);
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

    Entry.Parts = { FText::FromString(SenderName), FText::FromString(Message) };
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
    Entry.Parts = { FText::FromString(Message) };
    Entry.Colors = {};
    Append(Entry);

    UE_LOG(LogMessages, Log, TEXT("[Sys] %s"), *Message);
}

void UUR_MessageHistory::OnMatchStateTagChanged(AUR_GameState* GS)
{
    FMessageHistoryEntry Entry;
    Entry.Time = FDateTime::Now();
    Entry.Type = MessageType::Match;
    Entry.Parts = { FText::FromName(GS->GetMatchStateTag().GetTagName()) };
    Entry.Colors.Empty();
    Append(Entry);

    //NOTE: no logging here, there is already Match/Sub/State logging in GameState class.
}

void UUR_MessageHistory::OnFrag(AUR_PlayerState* Victim, AUR_PlayerState* Killer, TSubclassOf<UDamageType> DmgType, const FGameplayTagContainer& Tags)
{
    if (!IsValid(Victim) && !IsValid(Killer))
    {
        return;
    }

    FMessageHistoryEntry Entry;
    Entry.Time = FDateTime::Now();
    Entry.Type = MessageType::Death;

    Entry.Parts = { FText::FromString(TEXT("{Killer} killed {Victim}")), FText::FromString(TEXT("Somebody")), FText::FromString(TEXT("Somebody")) };
    Entry.Colors = { FColor::White, FColor::White };

    if (!Killer || Killer == Victim)
    {
        // Suicide
        // TODO: DmgType->GetSuicideText()
        Entry.Parts[0] = FText::FromString(FString::Printf(TEXT("{Victim} suicided with %s"), DmgType ? *DmgType->GetName() : TEXT("???")));
    }
    else
    {
        // Kill
        // TODO: DmgType->GetKillText()
        Entry.Parts[0] = FText::FromString(FString::Printf(TEXT("{Killer} killed {Victim} with %s"), DmgType ? *DmgType->GetName() : TEXT("???")));
    }

    if (Killer)
    {
        Entry.Parts[1] = FText::FromString(Killer->GetPlayerName());
        Entry.Colors[0] = UUR_FunctionLibrary::GetPlayerDisplayTextColor(Killer);
    }

    if (Victim)
    {
        Entry.Parts[2] = FText::FromString(Victim->GetPlayerName());
        Entry.Colors[1] = UUR_FunctionLibrary::GetPlayerDisplayTextColor(Victim);
    }

    for (const FGameplayTag& Tag : Tags)
    {
        // HMMM
        Entry.Parts.Add(FText::FromName(Tag.GetTagName()));
    }

    Append(Entry);

    FText FragLine = FText::FormatNamed(Entry.Parts[0], TEXT("Killer"), Entry.Parts[1], TEXT("Victim"), Entry.Parts[2]);
    if (Tags.Num() > 0)
    {
        UE_LOG(LogFrags, Log, TEXT("%s [%s]"), *FragLine.ToString(), *Tags.ToStringSimple());
    }
    else
    {
        UE_LOG(LogFrags, Log, TEXT("%s"), *FragLine.ToString());
    }
}

void UUR_MessageHistory::OnGlobalPickup(TSubclassOf<AUR_Pickup> PickupClass, AUR_PlayerState* Recipient)
{
    if (!IsValid(PickupClass) && !IsValid(Recipient))
    {
        return;
    }

    FMessageHistoryEntry Entry;
    Entry.Time = FDateTime::Now();
    Entry.Type = MessageType::Pickup;

    Entry.Parts = { FText::FromString(TEXT("{Recipient} has {Pickup}")), FText::FromString(TEXT("Somebody")), FText::FromString(TEXT("Something")) };
    Entry.Colors = { FColor::White };

    if (Recipient)
    {
        Entry.Parts[1] = FText::FromString(Recipient->GetPlayerName());
        Entry.Colors[0] = UUR_FunctionLibrary::GetPlayerDisplayTextColor(Recipient);
    }

    if (PickupClass)
    {
        AUR_Pickup* CDO = PickupClass->GetDefaultObject<AUR_Pickup>();
        Entry.Parts[2] = CDO->DisplayName;
    }

    Append(Entry);

    UE_LOG(LogPickups, Log, TEXT("%s"), *FText::FormatNamed(Entry.Parts[0], TEXT("Recipient"), Entry.Parts[1], TEXT("Pickup"), Entry.Parts[2]).ToString());
}

void UUR_MessageHistory::OnCharacterPickup(AUR_Pickup* Pickup)
{
    if (!IsValid(Pickup))
    {
        return;
    }

    // If pickup is broadcasting a global event, avoid adding a duplicate entry
    if (Pickup->bBroadcastPickupEvent)
    {
        return;
    }

    FMessageHistoryEntry Entry;
    Entry.Time = FDateTime::Now();
    Entry.Type = MessageType::Pickup;

    // NOTE: Keep same amount of parts as OnGlobalPickup, to make things simpler for BP.
    Entry.Parts = { FText::FromString(TEXT("Picked up {Pickup}")), FText::GetEmpty(), FText::FromString(TEXT("Something")) };
    Entry.Colors = { FColor::White };

    if (Pickup)
    {
        Entry.Parts[2] = Pickup->DisplayName;
    }

    Append(Entry);

    UE_LOG(LogPickups, Log, TEXT("%s"), *FText::FormatNamed(Entry.Parts[0], TEXT("Pickup"), Entry.Parts[2]).ToString());
}
