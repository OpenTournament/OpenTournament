// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_LocalPlayer.h"

#include "UR_MessageHistory.h"

#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_LocalPlayer)

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_LocalPlayer::UUR_LocalPlayer()
    : Super()
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
#if WITH_EDITOR
    if (GetWorld() && GetWorld()->IsPlayInEditor())
    {
        switch (GetWorld()->GetNetMode())
        {
            case NM_DedicatedServer:
            case NM_ListenServer:
            {
                return "Server";
            }
            case NM_Client:
            {
                if (GPlayInEditorID == 1)
                {
                    return PlayerName;
                }
                else
                {
                    return FString::Printf(TEXT("Client %d"), static_cast<int>(GPlayInEditorID));
                }
            }

            case NM_Standalone:
            {
                return PlayerName;
            }
            default:
            {
                return PlayerName;
            }
        }
    }
#endif WITH_EDITOR

    return PlayerName;
}
