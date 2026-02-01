// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"
#include "Styling/SlateWidgetStyleAsset.h"
#include "Styling/SlateWidgetStyleContainerBase.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Accessibility/SlateWidgetAccessibleTypes.h"

#define UE_API GAMESUBTITLES_API

class FText;
struct FSlateBrush;

/**
 * A widget that's used for displaying a subtitle somewhere on the viewport
 */
class SSubtitleDisplay : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SSubtitleDisplay)
		: _TextStyle( &FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText") )
		, _WrapTextAt(0.f)
		, _ManualSubtitles(false)
		{}

	SLATE_STYLE_ARGUMENT( FTextBlockStyle, TextStyle )

	/** Whether text wraps onto a new line when it's length exceeds this width; if this value is zero or negative, no wrapping occurs. */
	SLATE_ATTRIBUTE( float, WrapTextAt )

	SLATE_ATTRIBUTE(bool, ManualSubtitles )

	SLATE_END_ARGS()

	UE_API ~SSubtitleDisplay();

	UE_API void Construct( const FArguments& InArgs );

	UE_API void SetTextStyle(const FTextBlockStyle& InTextStyle);

	UE_API void SetBackgroundBrush(const FSlateBrush* InSlateBrush);

	UE_API void SetCurrentSubtitleText(const FText& SubtitleText);

	UE_API bool HasSubtitles() const;

	/** See WrapTextAt attribute */
	UE_API void SetWrapTextAt(const TAttribute<float>& InWrapTextAt);

private:
	void HandleSubtitleChanged(const FText& SubtitleText);

private:

	TSharedPtr<class SBorder> Background;

	/** The actual widget that will display the subtitle text */
	TSharedPtr<class SRichTextBlock> TextDisplay;
};

#undef UE_API
