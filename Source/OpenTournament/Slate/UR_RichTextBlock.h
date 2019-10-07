// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/RichTextBlock.h"
#include "UR_RichTextBlock.generated.h"

/**
 * UE 4.23 now provides builtin default text style property,
 * so this class hardly does anything anymore.
 *
 * Still, we will keep it with the following convenience points :
 * - bOverrideDefaultStyle true by default
 * - initialize default style with sensible defaults (taken from UTextBlock)
 * - add our CustomStyle decorator by default
 */
UCLASS()
class OPENTOURNAMENT_API UUR_RichTextBlock : public URichTextBlock
{
	GENERATED_BODY()

	UUR_RichTextBlock(const FObjectInitializer& ObjectInitializer);
};
