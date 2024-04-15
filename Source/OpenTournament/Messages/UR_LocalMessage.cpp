// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_LocalMessage.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/KismetSystemLibrary.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_LocalMessage::K2_ClientReceive_Implementation(APlayerController* LocalPC, int32 Switch, APlayerState* RelatedPlayer1, APlayerState* RelatedPlayer2, UObject* OptionalObject) const
{
    UKismetSystemLibrary::PrintString(LocalPC, FString::Printf(TEXT("%s Switch=%i P1=%s P2=%s Obj=%s"),
        *GetNameSafe(this),
        Switch,
        RelatedPlayer1 ? *RelatedPlayer1->GetPlayerName() : TEXT(""),
        RelatedPlayer2 ? *RelatedPlayer2->GetPlayerName() : TEXT(""),
        *GetNameSafe(OptionalObject)
    ));
}
