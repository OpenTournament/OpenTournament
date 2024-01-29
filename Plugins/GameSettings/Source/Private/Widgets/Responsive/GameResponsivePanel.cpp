// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameResponsivePanel.h"
#include "GameResponsivePanelSlot.h"
#include "Widgets/Responsive/SGameResponsivePanel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameResponsivePanel)

#define LOCTEXT_NAMESPACE "GameSetting"

/////////////////////////////////////////////////////
// UGameResponsivePanel

UGameResponsivePanel::UGameResponsivePanel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsVariable = false;
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UGameResponsivePanel::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MyGameResponsivePanel.Reset();
}

UClass* UGameResponsivePanel::GetSlotClass() const
{
	return UGameResponsivePanelSlot::StaticClass();
}

void UGameResponsivePanel::OnSlotAdded(UPanelSlot* InSlot)
{
	// Add the child to the live canvas if it already exists
	if ( MyGameResponsivePanel.IsValid() )
	{
		CastChecked<UGameResponsivePanelSlot>(InSlot)->BuildSlot(MyGameResponsivePanel.ToSharedRef());
	}
}

void UGameResponsivePanel::OnSlotRemoved(UPanelSlot* InSlot)
{
	// Remove the widget from the live slot if it exists.
	if ( MyGameResponsivePanel.IsValid() && InSlot->Content)
	{
		TSharedPtr<SWidget> Widget = InSlot->Content->GetCachedWidget();
		if ( Widget.IsValid() )
		{
			MyGameResponsivePanel->RemoveSlot(Widget.ToSharedRef());
		}
	}
}

UGameResponsivePanelSlot* UGameResponsivePanel::AddChildToGameResponsivePanel(UWidget* Content)
{
	return Cast<UGameResponsivePanelSlot>( Super::AddChild(Content) );
}

TSharedRef<SWidget> UGameResponsivePanel::RebuildWidget()
{
	MyGameResponsivePanel = SNew(SGameResponsivePanel);

	MyGameResponsivePanel->EnableVerticalStacking(bCanStackVertically);

	for ( UPanelSlot* PanelSlot : Slots )
	{
		if ( UGameResponsivePanelSlot* TypedSlot = Cast<UGameResponsivePanelSlot>(PanelSlot) )
		{
			TypedSlot->Parent = this;
			TypedSlot->BuildSlot(MyGameResponsivePanel.ToSharedRef());
		}
	}

	return MyGameResponsivePanel.ToSharedRef();
}

#if WITH_EDITOR

const FText UGameResponsivePanel::GetPaletteCategory()
{
	return LOCTEXT("Panel", "Panel");
}

#endif

/////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE

