// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Engine/DataAsset.h>

#include <GameplayTagContainer.h>

#include "UR_DamagePopStyle.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UObject;
class UStaticMesh;

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class UUR_DamagePopStyle : public UDataAsset
{
    GENERATED_BODY()

public:
    UUR_DamagePopStyle();

    UPROPERTY(EditDefaultsOnly, Category="DamagePop")
    FString DisplayText;

    UPROPERTY(EditDefaultsOnly, Category="DamagePop")
    FGameplayTagQuery MatchPattern;

    UPROPERTY(EditDefaultsOnly, Category="DamagePop", meta=(EditCondition=bOverrideColor))
    FLinearColor Color;

    UPROPERTY(EditDefaultsOnly, Category="DamagePop", meta=(EditCondition=bOverrideColor))
    FLinearColor CriticalColor;

    UPROPERTY(EditDefaultsOnly, Category="DamagePop", meta=(EditCondition=bOverrideMesh))
    TObjectPtr<UStaticMesh> TextMesh;

    UPROPERTY()
    bool bOverrideColor = false;

    UPROPERTY()
    bool bOverrideMesh = false;
};
