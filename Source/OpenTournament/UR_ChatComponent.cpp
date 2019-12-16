// Copyright 2019-2020 Open Tournament Project, All Rights Reserved.


#include "UR_ChatComponent.h"

#include "GameFramework/Controller.h"
#include "GameFramework/PlayerState.h"
#include "UR_GameModeBase.h"

#include "UR_GameState.h"
#include "UR_Character.h"
#include "UR_HealthComponent.h"
#include "UR_FunctionLibrary.h"

/**
* NOTE: It would kind of make sense to have a BaseChatComponent base class,
* and a PlayerChatComponent subclass that implements the Controller/PlayerState related parts and replicates.
*
* But it is kind of superfluous right now, and maybe ever.
* It's not like the player part is adding a tremendous amount of code.
* Nor that this thing will be widely used for non players entities.
*/

UUR_ChatComponent::UUR_ChatComponent()
    : OwnerController(nullptr)
    , FallbackOwnerName(TEXT("THING"))
    , AntiSpamDelay(1.f)
    , LastSendTime(0)
    , MaxMessageLength(150)
{
    SetIsReplicatedByDefault(true);
}

void UUR_ChatComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerController = Cast<AController>(GetOwner());

    if (AUR_GameModeBase* GM = GetWorld()->GetAuthGameMode<AUR_GameModeBase>())
    {
        GM->RegisterChatComponent(this);
    }
}

void UUR_ChatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    if (AUR_GameModeBase* GM = GetWorld()->GetAuthGameMode<AUR_GameModeBase>())
    {
        GM->UnregisterChatComponent(this);
    }
}

void UUR_ChatComponent::Send(const FString& Message, bool bTeamMessage)
{
    FString ValidatedMessage = Validate(ProcessChatParameters(Message), bTeamMessage);
    if (!ValidatedMessage.IsEmpty())
    {
        LastSendTime = GetWorld()->RealTimeSeconds;

        if (GetOwnerRole() == ROLE_Authority)
        {
            Broadcast(ValidatedMessage, bTeamMessage ? GetTeamIndex() : CHAT_INDEX_GLOBAL);
        }
        else
        {
            ServerSend(ValidatedMessage, bTeamMessage);
        }
    }
}

bool UUR_ChatComponent::ServerSend_Validate(const FString& Message, bool bTeamMessage)
{
    return true;
}

void UUR_ChatComponent::ServerSend_Implementation(const FString& Message, bool bTeamMessage)
{
    Send(Message, bTeamMessage);
}

FString UUR_ChatComponent::Validate_Implementation(const FString& Message, bool bTeamMessage)
{
    // anti spam
    if (GetWorld()->RealTimeSeconds - LastSendTime < AntiSpamDelay)
    {
        return TEXT("");
    }

    // trim
    FString ValidatedMessage = Message.TrimStartAndEnd();

    // crop message length
    if (ValidatedMessage.Len() > MaxMessageLength)
    {
        ValidatedMessage = ValidatedMessage.Left(MaxMessageLength-3).Append(TEXT("..."));	// TEXT('…') not supported by font atm
    }

    return ValidatedMessage;
}

void UUR_ChatComponent::Broadcast(const FString& Message, int32 TeamIndex)
{
    AUR_GameModeBase* GM = GetWorld()->GetAuthGameMode<AUR_GameModeBase>();
    if (GM)
    {
        for (auto Recipient : GM->ChatComponents)
        {
            if (Recipient->ShouldReceive(this, TeamIndex))
            {
                Recipient->Receive(this, Message, TeamIndex);
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No gamemode available to broadcast message"));
    }
}

bool UUR_ChatComponent::ShouldReceive_Implementation(UUR_ChatComponent* Sender, int32 TeamIndex)
{
    if (TeamIndex >= 0)
    {
        return GetTeamIndex() == TeamIndex;
    }

    if (TeamIndex == CHAT_INDEX_GLOBAL)
    {
        return true;
    }

    if (TeamIndex == CHAT_INDEX_SPEC)
    {
        return OwnerController && OwnerController->PlayerState && OwnerController->PlayerState->bOnlySpectator;
    }

    return false;
}

void UUR_ChatComponent::Receive_Implementation(UUR_ChatComponent* Sender, const FString& Message, int32 TeamIndex)
{
    FString SenderName = Sender ? Sender->GetOwnerName() : TEXT("");
    APlayerState* SenderPS = Sender ? Sender->GetPlayerState() : nullptr;

    ClientReceive(SenderName, Message, TeamIndex, SenderPS);

    // ClientReceive triggers dispatcher on client side. Avoid triggering twice in standalone.
    if ( GetNetMode() != NM_Standalone )
    {
        OnReceiveChatMessage.Broadcast(SenderName, Message, TeamIndex, SenderPS);
    }
}

int32 UUR_ChatComponent::GetTeamIndex_Implementation()
{
    APlayerState* PS = GetPlayerState();
    //TODO: return actual team index
    return PS ? (PS->bOnlySpectator ? CHAT_INDEX_SPEC : 0) : CHAT_INDEX_GLOBAL;
}

FString UUR_ChatComponent::GetOwnerName_Implementation()
{
    APlayerState* PS = GetPlayerState();
    return PS ? PS->GetPlayerName() : FallbackOwnerName;
}

APlayerState* UUR_ChatComponent::GetPlayerState_Implementation()
{
    return OwnerController ? OwnerController->PlayerState : nullptr;
}

void UUR_ChatComponent::ClientReceive_Implementation(const FString& SenderName, const FString& Message, int32 TeamIndex, APlayerState* SenderPS)
{
    OnReceiveChatMessage.Broadcast(SenderName, Message, TeamIndex, SenderPS);
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

    if (OwnerController)
    {
        AUR_Character* Char = Cast<AUR_Character>(OwnerController->GetCharacter());
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

FColor UUR_ChatComponent::GetChatMessageColor(UObject* WorldContextObject, int32 TeamIndex)
{
    if (TeamIndex >= 0)
    {
        // TODO: team color
        // possibly something like GameState->Teams[TeamIndex]->GetDisplayTextColor()

        //AUR_GameState* GS = WorldContextObject->GetWorld()->GetGameState<AUR_GameState>();

        return FColorList::Red;
    }
    else if (TeamIndex == CHAT_INDEX_SPEC)
        return UUR_FunctionLibrary::GetSpectatorDisplayTextColor();
    else
        return FColor::White;
}
