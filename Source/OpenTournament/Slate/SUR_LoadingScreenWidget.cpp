// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "SUR_LoadingScreenWidget.h"
#include "SlateOptMacros.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SThrobber.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SUR_LoadingScreenWidget::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		// 1/4
		+ SVerticalBox::Slot()
		.FillHeight(1)
		.HAlign(HAlign_Center)
		[
			// get bottom
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Bottom)
			[
				SNew(STextBlock)
				.TextStyle(FCoreStyle::Get(), "EmbossedText")
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
				.Text(FText::FromString(FString::Printf(TEXT("Loading %s"), *InArgs._MapName)))
			]
		]
		// 2/4
		+ SVerticalBox::Slot()
		.FillHeight(1)
		.HAlign(HAlign_Center)
		[
			// get bottom
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Bottom)
			[
				SNew(SThrobber)
			]
		]
		// lower half
		+ SVerticalBox::Slot()
		.FillHeight(1)
		+ SVerticalBox::Slot()
		.FillHeight(1)
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
