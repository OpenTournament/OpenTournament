// Fill out your copyright notice in the Description page of Project Settings.

#include "UR_Widget_KeyBindingMenu.h"

UUR_Widget_KeyBindingMenu::UUR_Widget_KeyBindingMenu(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	
}

//Should check these again when we decide how our menu system will work
void UUR_Widget_KeyBindingMenu::OnCloseButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("CloseButton Clicked"));
	/*
	OwningPlayer->SetInputMode(FInputModeGameOnly());
	OwningPlayer->bShowMouseCursor = false;
	RemoveFromViewport();
	*/
	Close();
}

//TODO: remove
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

void UUR_Widget_KeyBindingMenu::CreateKeyBindObject(FName Name, FKey Key)
{
	UUR_Object_KeyBind * NewKeyBind = NewObject<UUR_Object_KeyBind>(this);
	NewKeyBind->Name = Name;
	NewKeyBind->Key = Key;

	ControlsList->AddItem(NewKeyBind);
}

void UUR_Widget_KeyBindingMenu::PopulateKeyBindingList()
{
	AUR_PlayerController * Player = Cast<AUR_PlayerController>(GetOwningPlayer());
	UUR_PlayerInput * PlayerInput = Player->GetPlayerInput();
	const UInputSettings * Settings = GetDefault<UInputSettings>();

	for (TIndexedContainerIterator<TArray<FName>, FName, int32> it = AxisNames.CreateIterator(); it; it++)
	{
		TArray<FInputAxisKeyMapping> AxisMappingsForName;
		FName AxisName = AxisNames[it.GetIndex()];
		Settings->GetAxisMappingByName(AxisName, AxisMappingsForName);

		if (AxisMappingsForName.Num() > 0)
		{
			FInputAxisKeyMapping AxisMapping = AxisMappingsForName[0];
			CreateKeyBindObject(AxisMapping.AxisName, AxisMapping.Key);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("No axis with that name found. Name was: %s"), *AxisName.ToString());
		}
	}

	for (TIndexedContainerIterator<TArray<FName>, FName, int32> it = ActionNames.CreateIterator(); it; it++)
	{
		TArray<FInputActionKeyMapping> ActionMappingsForName;
		FName ActionName = ActionNames[it.GetIndex()];
		Settings->GetActionMappingByName(ActionName, ActionMappingsForName);

		if (ActionMappingsForName.Num() > 0)
		{
			FInputActionKeyMapping ActionMapping = ActionMappingsForName[0];
			CreateKeyBindObject(ActionMapping.ActionName, ActionMapping.Key);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("No action with that name found. Name was: %s"), *ActionName.ToString());
		}
	}
}
