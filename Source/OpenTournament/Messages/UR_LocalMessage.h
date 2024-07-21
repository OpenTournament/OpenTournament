// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/LocalMessage.h"

#include "UR_LocalMessage.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class APlayerController;
class APlayerState;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
* Engine struct is not exposed to BP...
*/
USTRUCT(BlueprintType)
struct FLocalMessageData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    APlayerController* LocalPC;

    UPROPERTY(BlueprintReadOnly)
    int32 Switch;

    UPROPERTY(BlueprintReadOnly)
    APlayerState* RelatedPlayer1;

    UPROPERTY(BlueprintReadOnly)
    APlayerState* RelatedPlayer2;

    UPROPERTY(BlueprintReadOnly)
    UObject* OptionalObject;

    FLocalMessageData()
        : LocalPC(nullptr)
        , Switch(0)
        , RelatedPlayer1(nullptr)
        , RelatedPlayer2(nullptr)
        , OptionalObject(nullptr)
    {
    }
};

/**
* Not sure how we are gonna structure this yet, in combination with HUD showing these messages...
*
* Definitely want support for RichText here, so we can colorize player names in frag messages etc.
*
* At some point the HUD wants to display this message and we need to resolve the final RichText string.
* As well as the duration of the message, size, some animation modes (fade in/out, scale etc.)
*
* Ideally all these props should be defaultproperties but also resolvable depending on Switch.
*
* If we queue announcements, this means HUD might display this message later after receiving.
* But we need to resolve stuff ASAP when receiving, otherwise a player might be gone or game status might have changed.
* This means we have to resolve all those properties upon receiving, and store them until HUD wants to display.
* We cannot store here because LocalMessage framework uses CDO only.
* Two possibilities :
* 1. Make a new struct to store them, store arrays of structs on HUD. This class becomes just a fleeting bridge.
* 2. Change behavior to actually instantiate the LocalMessage. Keep them in HUD until we don't need them anymore.
*
* I think we can do potentially interesting thing by instantiating the LocalMessage.
* Have events like WillShow() and Gone(). Maybe hook stuff there ?
* Use timers to do stuff slightly after receiving or displaying ?
*/

/**
* Other possibility : we don't have to use the LocalMessage system at all.
* We might be better off declaring appropriate functions in PlayerController,
* such as ReceiveFragMessage(), ReceivePickupMessage(), ReceiveGameMessage(), etc.
* With the appropriate parameters, then forward to dispatchers, which widgets/announcers can listen to.
* Then it is the widgets that decide how to format and display those messages.
*/

// @! TODO Probably deprecate this class and prefer GameplayMessageSubsystem
/**
 *
 */
UCLASS(Blueprintable)
class OPENTOURNAMENT_API UUR_LocalMessage : public ULocalMessage
{
    GENERATED_BODY()

    virtual void ClientReceive(const FClientReceiveData& ClientData) const override
    {
        K2_ClientReceive(
            ClientData.LocalPC,
            ClientData.MessageIndex,
            ClientData.RelatedPlayerState_1,
            ClientData.RelatedPlayerState_2,
            ClientData.OptionalObject
        );
    }

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic)
    void K2_ClientReceive(APlayerController* LocalPC, int32 Switch, APlayerState* RelatedPlayer1, APlayerState* RelatedPlayer2, UObject* OptionalObject) const;
};
