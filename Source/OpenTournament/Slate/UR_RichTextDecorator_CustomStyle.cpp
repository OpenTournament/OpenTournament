// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_RichTextDecorator_CustomStyle.h"

#include "Components/RichTextBlock.h"
#include "Framework/Text/SlateTextRun.h"

#include "UR_FunctionLibrary.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// FUR_RichTextDecorator_CustomStyle

class FRichTextDecorator_CustomStyle : public ITextDecorator
{
private:

    URichTextBlock* Owner;

public:

    FRichTextDecorator_CustomStyle(URichTextBlock* InOwner)
        : Owner(InOwner)
    {
    }

    virtual bool Supports(const FTextRunParseResults& RunParseResult, const FString& Text) const override
    {
        return (RunParseResult.Name == TEXT("S"));
    }

protected:

    virtual TSharedRef<ISlateRun> Create(const TSharedRef<class FTextLayout>& TextLayout, const FTextRunParseResults& RunParseResult, const FString& OriginalText, const TSharedRef< FString >& InOutModelText, const ISlateStyle* Style) override
    {
        FTextRange ModelRange;
        ModelRange.BeginIndex = InOutModelText->Len();

        // copy default text style
        FTextBlockStyle TextStyle(Owner->GetCurrentDefaultTextStyle());

        const FRunInfo RunInfo(RunParseResult.Name);
        for (const TPair<FString, FTextRange>& Pair : RunParseResult.MetaData)
        {
            // This is where we iterate through passed in attributes
            const FString& Key = Pair.Key;
            const FString& Value = OriginalText.Mid(Pair.Value.BeginIndex, Pair.Value.EndIndex - Pair.Value.BeginIndex);

            if (Key == TEXT("Color"))
            {
                FColor Color = FColor::FromHex(Value);
                FSlateColor SlateColor(Color.ReinterpretAsLinear());
                TextStyle.SetColorAndOpacity(SlateColor);
            }
            else if (Key == TEXT("Typeface"))
            {
                TextStyle.SetTypefaceFontName(FName(*Value));
            }
        }

        *InOutModelText += OriginalText.Mid(RunParseResult.ContentRange.BeginIndex, RunParseResult.ContentRange.EndIndex - RunParseResult.ContentRange.BeginIndex);
        ModelRange.EndIndex = InOutModelText->Len();

        return FSlateTextRun::Create(RunInfo, InOutModelText, TextStyle, ModelRange);
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////
// UUR_RichTextDecorator_CustomStyle

TSharedPtr<ITextDecorator> UUR_RichTextDecorator_CustomStyle::CreateDecorator(URichTextBlock* InOwner)
{
    return MakeShareable(new FRichTextDecorator_CustomStyle(InOwner));
}


FString UUR_RichTextDecorator_CustomStyle::DecorateRichText(const FString& InText, bool bColorize, const FColor& Color, const FString& Typeface, bool bNoEscape)
{
    if (InText.Len() == 0)
        return TEXT("");

    FString Result(TEXT("<S"), InText.Len() + 4);

    if (bColorize)
        Result.Append(FString::Printf(TEXT(" Color=\"%s\""), *Color.ToHex()));

    if (Typeface.Len() > 0)
        Result.Append(FString::Printf(TEXT(" Typeface=\"%s\""), *Typeface));

    Result.Append(FString::Printf(TEXT(">%s</>"), bNoEscape ? *InText : *UUR_FunctionLibrary::EscapeForRichText(InText)));

    return Result;
}
