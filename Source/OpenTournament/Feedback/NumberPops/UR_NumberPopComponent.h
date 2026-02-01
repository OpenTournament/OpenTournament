// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Components/ControllerComponent.h>

#include <GameplayTagContainer.h>

#include "UR_NumberPopComponent.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UObject;
struct FFrame;

/////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FGameNumberPopRequest
{
    GENERATED_BODY()

    // The world location to create the number pop at
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Number Pops")
    FVector WorldLocation;

    // Tags related to the source/cause of the number pop (for determining a style)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Number Pops")
    FGameplayTagContainer SourceTags;

    // Tags related to the target of the number pop (for determining a style)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game|Number Pops")
    FGameplayTagContainer TargetTags;

    // The number to display
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Number Pops")
    int32 NumberToDisplay = 0;

    // Whether the number is 'critical' or not (@TODO: move to a tag)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyra|Number Pops")
    bool bIsCriticalDamage = false;

    FGameNumberPopRequest()
        : WorldLocation(ForceInitToZero)
    {}
};

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(Abstract)
class UUR_NumberPopComponent : public UControllerComponent
{
    GENERATED_BODY()

public:
    UUR_NumberPopComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    /** Adds a damage number to the damage number list for visualization */
    UFUNCTION(BlueprintCallable, Category = Foo)
    virtual void AddNumberPop(const FGameNumberPopRequest& NewRequest)
    {}
};
