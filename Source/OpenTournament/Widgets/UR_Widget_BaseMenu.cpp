// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Widget_BaseMenu.h"

#include "WidgetBlueprintLibrary.h"
#include "UR_FunctionLibrary.h"

UUR_Widget_BaseMenu::UUR_Widget_BaseMenu(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ParentMenu = nullptr;

	bAutoPause = true;
	bPausedByDialog = false;

	bCloseOnEscape = true;
	CloseOnAction = TEXT("");

	// We should never let focus go back to parent widget or we are doomed.
	// In case we are making a dialog/popup -like thing (not full screen),
	// make sure to at least add a full screen invisible border in the background.
	// In these cases anyways it is good practice to cover the background with a shade at least.
	bIsFocusable = true;
}

void UUR_Widget_BaseMenu::Show(UWidget* InParentMenu)
{
	ParentMenu = InParentMenu;

	AddToViewport(10);	// base all menus on a higher zOrder so they don't conflict with hud widgets (chatbox...)

	UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(GetOwningPlayer(), this);	//<-- should take focus

	if (!IsValid(ParentMenu))
	{
		if (bAutoPause && !GetWorld()->IsPaused())
		{
			bPausedByDialog = GetOwningPlayer()->SetPause(true);
		}
	}
}

void UUR_Widget_BaseMenu::Close(bool bForce)
{
	if (!bForce && PreventClose())
		return;

	if (!IsValid(ParentMenu))
	{
		UWidgetBlueprintLibrary::SetInputMode_GameOnly(GetOwningPlayer());

		if (bPausedByDialog)
		{
			GetOwningPlayer()->SetPause(false);
			bPausedByDialog = false;
		}
	}
	else
	{
		ParentMenu->SetUserFocus(GetOwningPlayer());
	}

	RemoveFromParent();

	OnMenuClosed.Broadcast();
}

bool UUR_Widget_BaseMenu::PreventClose_Implementation()
{
	return false;
}

FReply UUR_Widget_BaseMenu::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	UE_LOG(LogTemp, Log, TEXT("OnKeyDown %s @ %s"), *InKeyEvent.GetKey().GetDisplayName().ToString(), *GetDisplayNameBase().ToString());

	if (bCloseOnEscape && InKeyEvent.GetKey() == EKeys::Escape)
	{
		Close();
	}
	else if (CloseOnAction.Len() > 0 && UUR_FunctionLibrary::IsKeyMappedToAction(InKeyEvent.GetKey(), FName(*CloseOnAction)))
	{
		Close();
	}
	else
	{
		// fwd to UMG
		Super::NativeOnKeyDown(InGeometry, InKeyEvent);
	}

	// capture all
	return FReply::Handled();
}
