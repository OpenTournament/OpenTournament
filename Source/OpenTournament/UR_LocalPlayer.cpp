// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_LocalPlayer.h"

#include "UR_MessageHistory.h"

#include "Engine/World.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_LocalPlayer::UUR_LocalPlayer(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    // no need to create a history for the CDO
    if (!HasAnyFlags(RF_ClassDefaultObject))
    {
        MessageHistory = CreateDefaultSubobject<UUR_MessageHistory>(TEXT("MessageHistory"));
    }

    PlayerName = FString::Printf(TEXT("OTPlayer-%d"), FMath::RandRange(10, 9999));
}

/////////////////////////////////////////////////////////////////////////////////////////////////

FString UUR_LocalPlayer::GetNickname() const
{
    if (GetWorld() && GetWorld()->IsPlayInEditor())
    {
        switch (GetWorld()->GetNetMode())
        {
            case NM_DedicatedServer:
            case NM_ListenServer:
                return "Server";

            case NM_Client:
            {
                if (GPlayInEditorID == 1)
                    return PlayerName;
                else
                    return FString::Printf(TEXT("Client %d"), GPlayInEditorID);
            }
            
            case NM_Standalone:
                return PlayerName;
        }
    }
    return PlayerName;
}
