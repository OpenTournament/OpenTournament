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
	static FString DecorateRichText(const FString& InText, bool bColorize, const FColor& Color, const FString& Typeface = TEXT(""), bool bNoEscape = false);
};
