// Fill out your copyright notice in the Description page of Project Settings.


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
