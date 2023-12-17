// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_RichTextBlock.h"

#include "Components/TextBlock.h"

#include "UR_RichTextDecorator_CustomStyle.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_RichTextBlock::UUR_RichTextBlock(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer)
{
    DecoratorClasses.Add(UUR_RichTextDecorator_CustomStyle::StaticClass());

    if (!IsRunningDedicatedServer())
    {
        bOverrideDefaultStyle = true;

        UTextBlock* DefaultTextBlock = UTextBlock::StaticClass()->GetDefaultObject<UTextBlock>();

        auto Style = GetDefaultTextStyle();
        Style.Font = DefaultTextBlock->GetFont();
        Style.ColorAndOpacity = DefaultTextBlock->GetColorAndOpacity();
        Style.ShadowOffset = DefaultTextBlock->GetShadowOffset();
        Style.ShadowColorAndOpacity = DefaultTextBlock->GetShadowColorAndOpacity();
        SetDefaultTextStyle(Style);
    }
}
