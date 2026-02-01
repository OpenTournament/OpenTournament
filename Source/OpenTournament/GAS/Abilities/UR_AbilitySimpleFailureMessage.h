// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"

#include "UR_AbilitySimpleFailureMessage.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_ABILITY_SIMPLE_FAILURE_MESSAGE);

/////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FGameAbilitySimpleFailureMessage
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite)
    TObjectPtr<APlayerController> PlayerController = nullptr;

    UPROPERTY(BlueprintReadWrite)
    FGameplayTagContainer FailureTags;

    UPROPERTY(BlueprintReadWrite)
    FText UserFacingReason;
};
