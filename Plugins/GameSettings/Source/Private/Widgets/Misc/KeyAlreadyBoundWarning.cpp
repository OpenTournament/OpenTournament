// Copyright Epic Games, Inc. All Rights Reserved.

#include "Widgets/Misc/KeyAlreadyBoundWarning.h"
#include "Components/TextBlock.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(KeyAlreadyBoundWarning)

void UKeyAlreadyBoundWarning::SetWarningText(const FText& InText)
{
	WarningText->SetText(InText);
}

void UKeyAlreadyBoundWarning::SetCancelText(const FText& InText)
{
	CancelText->SetText(InText);
}
