// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_GameModeBase.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_GameModeBase::AUR_GameModeBase(){}

void AUR_GameModeBase::RegisterChatComponent(UUR_ChatComponent* Comp)
{
	if (Comp)
		ChatComponents.AddUnique(Comp);
}

void AUR_GameModeBase::UnregisterChatComponent(UUR_ChatComponent* Comp)
{
	ChatComponents.Remove(Comp);
}
