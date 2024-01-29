// Copyright Epic Games, Inc. All Rights Reserved.

#include "Widgets/SSubtitleDisplay.h"

#include "Kismet/GameplayStatics.h"
#include "SubtitleManager.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/SRichTextBlock.h"

struct FSlateBrush;

void SSubtitleDisplay::Construct(const FArguments& InArgs)
{
	if (!InArgs._ManualSubtitles.Get())
	{
		FSubtitleManagerSetSubtitleText& OnSetSubtitleText = FSubtitleManager::GetSubtitleManager()->OnSetSubtitleText();
		OnSetSubtitleText.AddSP(this, &SSubtitleDisplay::HandleSubtitleChanged);
	}

	ChildSlot
	[
		SAssignNew(Background, SBorder)
		.Visibility(EVisibility::Collapsed)
		.Padding(FMargin(7.0, 5.0))
		[
			SAssignNew(TextDisplay, SRichTextBlock)
			.TextStyle(InArgs._TextStyle)
			.Justification(ETextJustify::Center)
			.WrapTextAt(InArgs._WrapTextAt)
		]
	];
}

SSubtitleDisplay::~SSubtitleDisplay()
{
	FSubtitleManager::GetSubtitleManager()->OnSetSubtitleText().RemoveAll(this);
}

void SSubtitleDisplay::SetTextStyle(const FTextBlockStyle& InTextStyle)
{
	TextDisplay->SetTextStyle(InTextStyle);
}

void SSubtitleDisplay::SetBackgroundBrush(const FSlateBrush* InSlateBrush)
{
	Background->SetBorderImage(InSlateBrush);
}

void SSubtitleDisplay::SetCurrentSubtitleText(const FText& InSubtitleText)
{
	Background->SetVisibility(InSubtitleText.IsEmpty() ? EVisibility::Collapsed : EVisibility::HitTestInvisible);
	TextDisplay->SetText(InSubtitleText);
}

bool SSubtitleDisplay::HasSubtitles() const
{
	return !TextDisplay->GetText().IsEmpty();
}

void SSubtitleDisplay::SetWrapTextAt(const TAttribute<float>& InWrapTextAt)
{
	TextDisplay->SetWrapTextAt(InWrapTextAt);
}

void SSubtitleDisplay::HandleSubtitleChanged(const FText& InSubtitleText)
{
	if (UGameplayStatics::AreSubtitlesEnabled())
	{
		Background->SetVisibility(InSubtitleText.IsEmpty() ? EVisibility::Collapsed : EVisibility::HitTestInvisible);
		TextDisplay->SetText(InSubtitleText);
	}
	else
	{
		Background->SetVisibility(EVisibility::Collapsed);
	}
}
