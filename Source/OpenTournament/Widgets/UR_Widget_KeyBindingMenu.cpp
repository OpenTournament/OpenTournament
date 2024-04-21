// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Widget_KeyBindingMenu.h"

#include "Components/Button.h"
#include "Components/ListView.h"
#include "GameFramework/InputSettings.h"

#include "Data/UR_Object_KeyBind.h"
#include "UR_BasePlayerController.h"
#include "UR_PlayerInput.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UUR_PlayerInput;

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_Widget_KeyBindingMenu::UUR_Widget_KeyBindingMenu(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , OwningPlayer(nullptr)
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
        CloseButton->OnClicked.AddDynamic(this, &ThisClass::OnCloseButtonClicked);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot bind CloseButton event: CloseButton pointer is null."));
    }

    PopulateKeyBindingList();
}

void UUR_Widget_KeyBindingMenu::CreateKeyBindObject(FName Name, FKey Key)
{
    UUR_Object_KeyBind* NewKeyBind = NewObject<UUR_Object_KeyBind>(this);
    NewKeyBind->Name = Name;
    NewKeyBind->Key = Key;

    ControlsList->AddItem(NewKeyBind);
}

void UUR_Widget_KeyBindingMenu::PopulateKeyBindingList()
{
    const UInputSettings* ProjectDefaults = GetDefault<UInputSettings>();

    const AUR_BasePlayerController* PC = Cast<AUR_BasePlayerController>(GetOwningPlayer());
    UUR_PlayerInput* UserSettings = PC->GetPlayerInput();

    TArray<FInputAxisKeyMapping> AxisMappings;
    for (const FName& AxisName : AxisNames)
    {
        if (UserSettings->FindUserAxisMappings(AxisName, AxisMappings))
        {
            CreateKeyBindObject(AxisName, AxisMappings[0].Key);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("No axis with that name found. Name was: %s"), *AxisName.ToString());
            CreateKeyBindObject(AxisName, FKey());
        }
    }

    TArray<FInputActionKeyMapping> ActionMappings;
    for (const FName& ActionName : ActionNames)
    {
        if (UserSettings->FindUserActionMappings(ActionName, ActionMappings))
        {
            CreateKeyBindObject(ActionName, ActionMappings[0].Key);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("No action with that name found. Name was: %s"), *ActionName.ToString());
            CreateKeyBindObject(ActionName, FKey());
        }
    }
}
