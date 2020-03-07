// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UR_Widget_BaseMenu.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMenuClosedSignature);

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Base class for menu widgets
 */
UCLASS()
class OPENTOURNAMENT_API UUR_Widget_BaseMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UUR_Widget_BaseMenu(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadWrite)
	UWidget* ParentMenu;

	/**
	* Menu will attempt to pause the game if it's shown with Parent=null.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = "true"))
	bool bAutoPause;
	bool bPausedByDialog;

	/**
	* Escape key will call Close().
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = "true"))
	bool bCloseOnEscape;

	/**
	* This specific action (from InputActions) will call Close().
	*/
	UPROPERTY(EditAnywhere)
	FString CloseOnAction;

	/**
	* Event dispatcher when menu is closed.
	*/
	UPROPERTY(BlueprintAssignable)
	FOnMenuClosedSignature OnMenuClosed;

	/**
	* Shows the menu. Adds to viewport. Switches input mode to UIOnly.
	*/
	UFUNCTION(BlueprintCallable)
	virtual void Show(UWidget* InParentMenu);

	/**
	* Closes the menu, unless bForce is false and PreventClose() returns true.
	* If parent is valid, focus is returned to parent.
	* Else, user input mode is set back to GameOnly.
	*/
	UFUNCTION(BlueprintCallable)
	virtual void Close(bool bForce = false);

	/**
	* Return true to prevent closing.
	* Use this to prompt confirmation when eg. there are unsaved changes or something.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool PreventClose();

	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
};
