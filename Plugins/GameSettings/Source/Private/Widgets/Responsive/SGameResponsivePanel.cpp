// Copyright Epic Games, Inc. All Rights Reserved.

#include "SGameResponsivePanel.h"

#include "Framework/Application/SlateApplication.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Layout/ArrangedChildren.h"
#include "Widgets/SViewport.h"

#define LOCTEXT_NAMESPACE "GameSetting"

//TODO Nick Darnell
// Hello.  It appears you've discovered this widget.
// This widget currently isn't being generally used.  I'm prototyping out some
// ideas for settings.  Talk to me.

SGameResponsivePanel::SGameResponsivePanel()
	: InnerGrid(SNew(SGridPanel))
{
	SetCanTick(false);
	bCanSupportFocus = false;
	bHasCustomPrepass = true;
	bHasRelativeLayoutScale = true;
	bCanWrapVertically = true;
}

void SGameResponsivePanel::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		InnerGrid
	];
}

SGridPanel::FSlot& SGameResponsivePanel::AddSlot()
{
	SGridPanel::FSlot* Slot;
	InnerGrid->AddSlot(InnerGrid->GetChildren()->Num(), 0)
		.Expose(Slot);
	InnerSlots.Add(Slot);

	RefreshLayout();

	return *Slot;
}

int32 SGameResponsivePanel::RemoveSlot(const TSharedRef<SWidget>& SlotWidget)
{
	for (int32 SlotIdx = 0; SlotIdx < InnerSlots.Num(); ++SlotIdx)
	{
		if (SlotWidget == InnerSlots[SlotIdx]->GetWidget())
		{
			InnerSlots.RemoveAt(SlotIdx);
			break;
		}
	}

	return InnerGrid->RemoveSlot(SlotWidget);
}

void SGameResponsivePanel::ClearChildren()
{
	InnerGrid->ClearChildren();
}

void SGameResponsivePanel::EnableVerticalStacking(const bool bCanVerticallyWrap)
{
	bCanWrapVertically = bCanVerticallyWrap;
}

bool SGameResponsivePanel::CustomPrepass(float LayoutScaleMultiplier)
{
	RefreshResponsiveness();
	return true;
}

void SGameResponsivePanel::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
	ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(
		ChildSlot.GetWidget(),
		FVector2D(0, 0),
		AllottedGeometry.GetLocalSize() / Scale,
		Scale
	));
}

FVector2D SGameResponsivePanel::ComputeDesiredSize(float InLayoutScale) const
{
	return SCompoundWidget::ComputeDesiredSize(InLayoutScale) * Scale;
}

float SGameResponsivePanel::GetRelativeLayoutScale(int32 ChildIndex, float LayoutScaleMultiplier) const
{
	return Scale;
}

bool SGameResponsivePanel::ShouldWrap() const
{
	if (PhysialScreenSize.IsZero() || !bCanWrapVertically)
	{
		return false;
	}

	return (PhysialScreenSize.X < 7);
}

void SGameResponsivePanel::RefreshResponsiveness()
{
	PhysialScreenSize = FVector2D(0, 0);

	TSharedPtr<SViewport> GameViewport = FSlateApplication::Get().GetGameViewport();
	if (GameViewport.IsValid())
	{
		TSharedPtr<ISlateViewport> ViewportInterface = GameViewport->GetViewportInterface().Pin();
		if (ViewportInterface.IsValid())
		{
			const FIntPoint ViewportSize = ViewportInterface->GetSize();

			int32 ScreenDensity = 0;
			FPlatformApplicationMisc::GetPhysicalScreenDensity(ScreenDensity);
			
			if (ScreenDensity != 0)
			{
				PhysialScreenSize = ViewportSize / (float)ScreenDensity;
			}
		}
	}

	const bool bShouldWrap = ShouldWrap();
	const float NewScale = bShouldWrap ? 1.5f : 1.0f;
	if (!FMath::IsNearlyEqual(NewScale, Scale))
	{
		Scale = NewScale;
		RefreshLayout();
		Invalidate(EInvalidateWidgetReason::Prepass);
	}
}

void SGameResponsivePanel::RefreshLayout()
{
	const bool bShouldWrap = ShouldWrap();

	InnerGrid->ClearFill();

	for (int32 SlotIdx = 0; SlotIdx < InnerSlots.Num(); ++SlotIdx)
	{
		InnerSlots[SlotIdx]->SetColumn(bShouldWrap ? 0 : SlotIdx);
		InnerSlots[SlotIdx]->SetRow(bShouldWrap ? SlotIdx : 0);

		if (!bShouldWrap)
		{
			InnerGrid->SetColumnFill(SlotIdx, 1.0f);
		}
	}

	if (bShouldWrap)
	{
		InnerGrid->SetColumnFill(0, 1.0f);
	}
}

/////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
