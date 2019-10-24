// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UR_Widget_BaseMenu.h"
#include "UR_WidgetChatHistory.generated.h"

/**
 * 
 */
UCLASS()
class OPENTOURNAMENT_API UUR_WidgetChatHistory : public UUR_Widget_BaseMenu
{
	GENERATED_BODY()

	UUR_WidgetChatHistory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
	{
		CloseOnAction = TEXT("ShowChatHistory");
	}
};
