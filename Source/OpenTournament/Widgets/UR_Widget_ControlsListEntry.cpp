// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Widget_ControlsListEntry.h"

#include "Components/InputKeySelector.h"
#include "Components/TextBlock.h"
#include "Data/UR_Object_KeyBind.h"

#include "UR_BasePlayerController.h"
#include "UR_PlayerInput.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UUR_Object_KeyBind;
class UUR_PlayerInput;
class AUR_BasePlayerController;

UUR_Widget_ControlsListEntry::UUR_Widget_ControlsListEntry(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    IsSelectingKey = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_Widget_ControlsListEntry::NativeConstruct()
{
    Super::NativeConstruct();
    EntryKeySelector->OnIsSelectingKeyChanged.AddDynamic(this, &ThisClass::OnEntryKeySelectorIsSelectingKeyChanged);
    EntryKeySelector->OnKeySelected.AddDynamic(this, &ThisClass::OnEntryKeySelectorKeyChanged);

    //LOCTABLE_FROMFILE_GAME("KeyBindingStringTable", "KeyBindingStringTable", "OpenTournament/StringTables/ST_KeyBindingNames.csv");
}

void UUR_Widget_ControlsListEntry::OnEntryKeySelectorKeyChanged(FInputChord SelectedKey)
{
    UE_LOG(LogTemp, Warning, TEXT("EntryKeySelector Key was Changed"));
    if (IsSelectingKey)
    {
        UE_LOG(LogTemp, Warning, TEXT("EntryKeySelector Key Changed (keybind modify"));
        AUR_BasePlayerController* PlayerController = Cast<AUR_BasePlayerController>(GetOwningPlayer());
        UUR_PlayerInput* PlayerInput = Cast<UUR_PlayerInput>(PlayerController->GetPlayerInput());
        UUR_Object_KeyBind* KeyBind = Cast<UUR_Object_KeyBind>(Item);

        /*FInputActionKeyMapping KeyMapping(KeyBind->ActionName, SelectedKey.Key);

        PlayerInput->ModifyActionKeyMapping(KeyBind->ActionName, KeyMapping);
        KeyBind->Key = SelectedKey.Key;*/
        if (PlayerInput->ModifyKeyMapping(KeyBind->Name, SelectedKey))
        {
            KeyBind->Key = SelectedKey.Key;
            UpdateEntry();
        }

        IsSelectingKey = false;
    }
}

void UUR_Widget_ControlsListEntry::OnEntryKeySelectorIsSelectingKeyChanged()
{
    UE_LOG(LogTemp, Warning, TEXT("EntryKeySelector IsSelectingKey Changed"));
    IsSelectingKey = true;
}

void UUR_Widget_ControlsListEntry::NativeOnListItemObjectSet(UObject* InObject)
{
    Item = InObject;
    UpdateEntry();
}

void UUR_Widget_ControlsListEntry::UpdateEntry() const
{
    if (const UUR_Object_KeyBind* KeyBind = Cast<UUR_Object_KeyBind>(Item.Get()))
    {
        // TODO: Use proper localization instead
        EntryName->SetText(FText::FromName(KeyBind->Name));
        EntryKeySelector->SetSelectedKey(FInputChord(KeyBind->Key));
    }
}
