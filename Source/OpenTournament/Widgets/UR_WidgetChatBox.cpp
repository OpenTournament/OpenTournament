// Copyright 2019-2020 Open Tournament Project, All Rights Reserved.


#include "UR_WidgetChatBox.h"

#include "VerticalBox.h"
#include "VerticalBoxSlot.h"

#include "Utilities/UR_Logging.h"

void UUR_WidgetChatBox::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	APlayerController* PC = Cast<APlayerController>(GetOwningPlayer());
	if (PC && PC->InputComponent)
	{
		PC->InputComponent->BindAction("BeginSay", IE_Pressed, this, &UUR_WidgetChatBox::OnBeginSay);
		PC->InputComponent->BindAction("BeginTeamSay", IE_Pressed, this, &UUR_WidgetChatBox::OnBeginTeamSay);
	}
}
