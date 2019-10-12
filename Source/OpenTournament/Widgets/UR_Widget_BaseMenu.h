// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UR_Widget_BaseMenu.generated.h"

/**
 * Base class for menu widgets
 */
UCLASS()
class OPENTOURNAMENT_API UUR_Widget_BaseMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	//Should be called when opening the menu
	//Override this in derived classes
	virtual void OpenMenu();
};
