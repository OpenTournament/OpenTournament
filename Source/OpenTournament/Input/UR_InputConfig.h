// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

#include "UR_InputConfig.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UInputAction;
class UObject;
struct FFrame;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * FGameInputAction
 *
 *	Struct used to map a input action to a gameplay input tag.
 */
USTRUCT(BlueprintType)
struct FGameInputAction
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<const UInputAction> InputAction = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "InputTag"))
    FGameplayTag InputTag;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * UUR_InputConfig
 *
 *	Non-mutable data asset that contains input configuration properties.
 */
UCLASS(BlueprintType, Const)
class UUR_InputConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    UUR_InputConfig(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintCallable, Category = "OT|Pawn")
    const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

    UFUNCTION(BlueprintCallable, Category = "OT|Pawn")
    const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

public:
    // List of input actions used by the owner.  These input actions are mapped to a gameplay tag and must be manually bound.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
    TArray<FGameInputAction> NativeInputActions;

    // List of input actions used by the owner.  These input actions are mapped to a gameplay tag and are automatically bound to abilities with matching input tags.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
    TArray<FGameInputAction> AbilityInputActions;
};
