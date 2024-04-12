// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_GameModeBase.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_GameModeBase::AUR_GameModeBase()
{
}

void AUR_GameModeBase::RegisterChatComponent(UUR_ChatComponent* InComponent)
{
    if (InComponent)
    {
        ChatComponents.AddUnique(InComponent);
    }
}

void AUR_GameModeBase::UnregisterChatComponent(UUR_ChatComponent* InComponent)
{
    if (ChatComponents.Num() > 0)
    {
        ChatComponents.Remove(InComponent);
    }
}
