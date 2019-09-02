// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Components/TextBlock.h"
#include "Components/InputKeySelector.h"
#include "Data/UR_Object_KeyBind.h"
#include "UR_Widget_ControlsListEntry.generated.h"

/**
 * 
 */
UCLASS()
class OPENTOURNAMENT_API UUR_Widget_ControlsListEntry : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	
public:
	UUR_Widget_ControlsListEntry(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(meta = (BindWidget))
		UTextBlock * EntryName;

	UPROPERTY(meta = (BindWidget))
		UInputKeySelector * EntryKeySelector;

	UObject * Item;

	UFUNCTION()
		void OnEntryKeySelectorKeyChanged(FInputChord SelectedKey);

	void NativeConstruct();
	void SetListItemObjectInternal(UObject* InObject);

private:
	void UpdateEntry();
};
