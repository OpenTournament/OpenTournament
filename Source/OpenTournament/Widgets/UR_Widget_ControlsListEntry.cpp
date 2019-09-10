// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Widget_ControlsListEntry.h"

UUR_Widget_ControlsListEntry::UUR_Widget_ControlsListEntry(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	IsSelectingKey = false;
}

void UUR_Widget_ControlsListEntry::OnEntryKeySelectorKeyChanged(FInputChord SelectedKey)
{
	AUR_PlayerController * PlayerController = static_cast<AUR_PlayerController*>(GetOwningPlayer());
	UUR_PlayerInput * PlayerInput = static_cast<UUR_PlayerInput*>(PlayerController->GetPlayerInput());
	UUR_Object_KeyBind * KeyBind = static_cast<UUR_Object_KeyBind*>(Item);

	FInputActionKeyMapping KeyMapping(KeyBind->ActionName, SelectedKey.Key);

	PlayerInput->ModifyActionKeyMapping(KeyBind->ActionName, KeyMapping);
}

void UUR_Widget_ControlsListEntry::OnEntryKeySelectorIsSelectingKeyChanged()
{
	IsSelectingKey = !IsSelectingKey;
}

void UUR_Widget_ControlsListEntry::NativeConstruct()
{
	Super::NativeConstruct();
	EntryKeySelector->OnKeySelected.AddDynamic(this, UUR_Widget_ControlsListEntry::OnEntryKeySelectorKeyChanged);
}

void UUR_Widget_ControlsListEntry::SetListItemObjectInternal(UObject * InObject)
{
	Item = InObject;
	UpdateEntry();
}

void UUR_Widget_ControlsListEntry::UpdateEntry()
{
	UUR_Object_KeyBind* KeyBind = static_cast<UUR_Object_KeyBind*>(Item);
	EntryName->SetText(FText::FromName(KeyBind->ActionName));
	EntryKeySelector->SetSelectedKey(FInputChord(KeyBind->Key));
}
