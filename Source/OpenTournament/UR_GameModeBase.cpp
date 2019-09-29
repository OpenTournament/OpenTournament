// Copyright 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_GameModeBase.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_GameModeBase::AUR_GameModeBase(){}

void AUR_GameModeBase::OpenControlsMenu()
{
	if (ControlsMenu == nullptr)
	{
		ControlsMenu = CreateWidget<UUR_Widget_BaseMenu>(GetWorld(), KeyBindingMenu);
	}
	ControlsMenu->OpenMenu();
}
