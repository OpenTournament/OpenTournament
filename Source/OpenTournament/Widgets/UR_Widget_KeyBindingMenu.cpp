// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Widget_KeyBindingMenu.h"

UUR_Widget_KeyBindingMenu::UUR_Widget_KeyBindingMenu(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	
}

void UUR_Widget_KeyBindingMenu::OnMyButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("MyButton Clicked"));
}

void UUR_Widget_KeyBindingMenu::OpenMenu()
{
	AddToViewport();
	if (MyButton != nullptr)
	{
		MyButton->OnClicked.AddDynamic(this, &UUR_Widget_KeyBindingMenu::OnMyButtonClicked);
	}
	else
		UE_LOG(LogTemp, Error, TEXT("Cannot bind MyButton event: MyButton pointer is null."));

	AUR_PlayerController * Player = static_cast<AUR_PlayerController*>(GetOwningPlayer());
	UUR_PlayerInput * PlayerInput = Player->GetPlayerInput();
	const UInputSettings * Settings = GetDefault<UInputSettings>();

	TArray<FName> ActionNames;
	Settings->GetActionNames(ActionNames);

	for (TIndexedContainerIterator<TArray<FName>, FName, int32> it = ActionNames.CreateIterator(); it; it++)
	{
		TArray<FInputActionKeyMapping> ActionMappingsForName;
		Settings->GetActionMappingByName(ActionNames[it.GetIndex()], ActionMappingsForName);

		FInputActionKeyMapping ActionMapping = ActionMappingsForName[0];
		UUR_Object_KeyBind * NewKeyBind = NewObject<UUR_Object_KeyBind>(this);
		NewKeyBind->ActionName = ActionMapping.ActionName;
		NewKeyBind->Key = ActionMapping.Key;

		ControlsList->AddItem(NewKeyBind);
	}
}
