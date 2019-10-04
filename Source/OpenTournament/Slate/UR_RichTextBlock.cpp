// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_RichTextBlock.h"
#include "UR_RichTextDecorator_CustomStyle.h"
#include "TextBlock.h"
#include "Widgets/Text/SRichTextBlock.h"

UUR_RichTextBlock::UUR_RichTextBlock(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DecoratorClasses.Add(UUR_RichTextDecorator_CustomStyle::StaticClass());

	if (!IsRunningDedicatedServer())
	{
		UTextBlock* DefaultTextBlock = UTextBlock::StaticClass()->GetDefaultObject<UTextBlock>();
		K2_DefaultTextStyle.Font = DefaultTextBlock->Font;
		K2_DefaultTextStyle.ColorAndOpacity = DefaultTextBlock->ColorAndOpacity;
		K2_DefaultTextStyle.ShadowOffset = DefaultTextBlock->ShadowOffset;
		K2_DefaultTextStyle.ShadowColorAndOpacity = DefaultTextBlock->ShadowColorAndOpacity;
	}
	DefaultTextStyle = K2_DefaultTextStyle;
}

void UUR_RichTextBlock::SynchronizeProperties()
{
	// Updates undecorated text, but not decorated parts because they are different widgets. Don't know how to get them.
	MyRichTextBlock->SetTextStyle(K2_DefaultTextStyle);

	Super::SynchronizeProperties();
}

void UUR_RichTextBlock::UpdateStyleData()
{
	Super::UpdateStyleData();

	DefaultTextStyle = K2_DefaultTextStyle;
}
