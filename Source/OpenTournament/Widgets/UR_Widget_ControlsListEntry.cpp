// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Widget_ControlsListEntry.h"

UUR_Widget_ControlsListEntry::UUR_Widget_ControlsListEntry(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	//Super(ObjectInitializer);
}

void UUR_Widget_ControlsListEntry::NativeConstruct()
{
	Super::NativeConstruct();
	EntryName->SetText(FText::FromString("Hello"));
	if (Item != nullptr)
	{
		
	}
}

void UUR_Widget_ControlsListEntry::OnEntryKeySelectorKeyChanged(FInputChord SelectedKey)
{
	//TODO: Add definition
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
