// Fill out your copyright notice in the Description page of Project Settings.

#include "UR_Widget_KeyBindingMenu.h"

UUR_Widget_KeyBindingMenu::UUR_Widget_KeyBindingMenu(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	
}

//Should check these again when we decide how our menu system will work
void UUR_Widget_KeyBindingMenu::OnCloseButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("CloseButton Clicked"));
	OwningPlayer->SetInputMode(FInputModeGameOnly());
	OwningPlayer->bShowMouseCursor = false;
	RemoveFromViewport();
}

void UUR_Widget_KeyBindingMenu::OpenMenu()
{
	OwningPlayer = GetOwningPlayer();
	OwningPlayer->SetInputMode(FInputModeUIOnly());
	OwningPlayer->bShowMouseCursor = true;
	AddToViewport();
}

void UUR_Widget_KeyBindingMenu::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (CloseButton != nullptr)
	{
		CloseButton->OnClicked.AddDynamic(this, &UUR_Widget_KeyBindingMenu::OnCloseButtonClicked);
	}
	else
		UE_LOG(LogTemp, Error, TEXT("Cannot bind CloseButton event: CloseButton pointer is null."));

	PopulateKeyBindingList();
}

void UUR_Widget_KeyBindingMenu::PopulateKeyBindingList()
{
	AUR_PlayerController * Player = static_cast<AUR_PlayerController*>(GetOwningPlayer());
	UUR_PlayerInput * PlayerInput = Player->GetPlayerInput();
	const UInputSettings * Settings = GetDefault<UInputSettings>();

	for (TIndexedContainerIterator<TArray<FName>, FName, int32> it = AxisNames.CreateIterator(); it; it++)
	{
		TArray<FInputAxisKeyMapping> AxisMappingsForName;
		Settings->GetAxisMappingByName(AxisNames[it.GetIndex()], AxisMappingsForName);

		FInputAxisKeyMapping AxisMapping = AxisMappingsForName[0];
		UUR_Object_KeyBind * NewKeyBind = NewObject<UUR_Object_KeyBind>(this);
		NewKeyBind->ActionName = AxisMapping.AxisName;
		NewKeyBind->Key = AxisMapping.Key;

		ControlsList->AddItem(NewKeyBind);
	}

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
