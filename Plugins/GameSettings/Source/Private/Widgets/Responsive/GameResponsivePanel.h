// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/PanelWidget.h"
#include "GameResponsivePanel.generated.h"

class UGameResponsivePanelSlot;

/**
 * Allows widgets to be laid out in a flow horizontally.
 *
 * * Many Children
 * * Flow Horizontal
 */
UCLASS()
class UGameResponsivePanel : public UPanelWidget
{
	GENERATED_UCLASS_BODY()

	/**  */
	UFUNCTION(BlueprintCallable, Category="Widget")
	UGameResponsivePanelSlot* AddChildToGameResponsivePanel(UWidget* Content);

#if WITH_EDITOR
	// UWidget interface
	virtual const FText GetPaletteCategory() override;
	// End UWidget interface
#endif

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
	bool bCanStackVertically = true;

protected:

	// UPanelWidget
	virtual UClass* GetSlotClass() const override;
	virtual void OnSlotAdded(UPanelSlot* Slot) override;
	virtual void OnSlotRemoved(UPanelSlot* Slot) override;
	// End UPanelWidget

protected:

	TSharedPtr<class SGameResponsivePanel> MyGameResponsivePanel;

protected:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface
};
