// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/RichTextBlockDecorator.h"
#include "UR_RichTextDecorator_CustomStyle.generated.h"


/**
 * 
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
