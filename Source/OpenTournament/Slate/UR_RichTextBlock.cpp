// Copyright 2019-2020 Open Tournament Project, All Rights Reserved.


#include "UR_RichTextBlock.h"
#include "UR_RichTextDecorator_CustomStyle.h"
#include "TextBlock.h"


UUR_RichTextBlock::UUR_RichTextBlock(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DecoratorClasses.Add(UUR_RichTextDecorator_CustomStyle::StaticClass());

	if (!IsRunningDedicatedServer())
	{
		bOverrideDefaultStyle = true;

		UTextBlock* DefaultTextBlock = UTextBlock::StaticClass()->GetDefaultObject<UTextBlock>();
		DefaultTextStyleOverride.Font = DefaultTextBlock->Font;
		DefaultTextStyleOverride.ColorAndOpacity = DefaultTextBlock->ColorAndOpacity;
		DefaultTextStyleOverride.ShadowOffset = DefaultTextBlock->ShadowOffset;
		DefaultTextStyleOverride.ShadowColorAndOpacity = DefaultTextBlock->ShadowColorAndOpacity;
	}
}
