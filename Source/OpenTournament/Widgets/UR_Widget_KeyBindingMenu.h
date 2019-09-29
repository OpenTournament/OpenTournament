// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/UR_Widget_BaseMenu.h"
#include "Components/Button.h"
#include "Components/ListView.h"
#include "UR_PlayerController.h"
#include "Data/UR_Object_KeyBind.h"
#include "UR_Widget_KeyBindingMenu.generated.h"

/**
 * 
 */
UCLASS()
class OPENTOURNAMENT_API UUR_Widget_KeyBindingMenu : public UUR_Widget_BaseMenu
{
	GENERATED_BODY()
	
public:
	UUR_Widget_KeyBindingMenu(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(meta = (BindWidget))
		UButton * CloseButton;

	UPROPERTY(meta = (BindWidget))
		UListView * ControlsList;

	UFUNCTION()
		void OnCloseButtonClicked();

	UFUNCTION(BlueprintCallable, Category="UnrealRemake|UI")
		void OpenMenu();

	void NativeOnInitialized();

private:
	UPROPERTY(EditDefaultsOnly)
	TArray<FName> AxisNames;
	UPROPERTY(EditDefaultsOnly)
	TArray<FName> ActionNames;

	APlayerController * OwningPlayer;

	void PopulateKeyBindingList();
};
