// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/RichTextBlock.h"
#include "UR_RichTextBlock.generated.h"

/**
 * Rich text block extension where default text style is a builtin property,
 * so we don't need to create additional StyleSet assets,
 * if we only intend to use the CustomStyle decorator.
 *
 * CustomStyle decorator is already added by default.
 */
UCLASS()
class OPENTOURNAMENT_API UUR_RichTextBlock : public URichTextBlock
{
	GENERATED_BODY()

	UUR_RichTextBlock(const FObjectInitializer& ObjectInitializer);

public:

	/**
	* Configurable default text style.
	*
	* Warning: decorated parts will not be updated in real time.
	* Recompile widget to update.
	*/
	UPROPERTY(EditAnywhere, Category = Appearance, Meta = (DisplayName = "Default Text Style"))
	FTextBlockStyle K2_DefaultTextStyle;

	/**
	* Returns a copy of the current default text style.
	*/
	UFUNCTION(BlueprintPure)
	FTextBlockStyle GetDefaultTextStyle() { return K2_DefaultTextStyle; }

	/**
	* Updates the default text style.
	*/
	UFUNCTION(BlueprintCallable)
	void SetDefaultTextStyle(const FTextBlockStyle& NewStyle)
	{
		K2_DefaultTextStyle = NewStyle;
		DefaultTextStyle = NewStyle;
		SynchronizeProperties();
	}

	// UWidget interface
	virtual void SynchronizeProperties() override;
	// End of UWidget interface

protected:

	virtual void UpdateStyleData() override;

};
