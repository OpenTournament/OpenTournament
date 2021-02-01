// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "Components/RichTextBlockDecorator.h"
#include "UR_RichTextDecorator_CustomStyle.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * RichTextDecorator
 */
UCLASS(BlueprintType)
class OPENTOURNAMENT_API UUR_RichTextDecorator_CustomStyle : public URichTextBlockDecorator
{
	GENERATED_BODY()

public:

	virtual TSharedPtr<ITextDecorator> CreateDecorator(URichTextBlock* InOwner) override;

	UFUNCTION(BlueprintPure)
	static FString DecorateRichString(const FString& InText, bool bColorize, const FColor& Color, const FString& Typeface = TEXT(""), bool bNoEscape = false);

    /** Convenience for FText */
    UFUNCTION(BlueprintPure)
    static FText DecorateRichText(const FText& InText, bool bColorize, const FColor& Color, const FString& Typeface = TEXT(""), bool bNoEscape = false)
    {
        return FText::FromString(DecorateRichString(InText.ToString(), bColorize, Color, Typeface, bNoEscape));
    }

};
