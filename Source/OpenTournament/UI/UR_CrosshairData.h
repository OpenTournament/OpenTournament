// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <CoreMinimal.h>
#include <GameplayTagContainer.h>
#include <Engine/DataAsset.h>

#include "UR_CrosshairData.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations

class UTexture2D;

/////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Crosshair Data Class
 */
UCLASS(BlueprintType, Blueprintable)
class OPENTOURNAMENT_API UUR_CrosshairData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UUR_CrosshairData();

    /////////////////////////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
    virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif // WITH_EDITOR

    /////////////////////////////////////////////////////////////////////////////////////////////////

    // Crosshair Tag
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FGameplayTag Tag;

    // @! TODO : Evaluate if we should be utilizing the Texture-Color-Scale approach or just simplify to a single Material with parameters
    // @! TODO : How do we handle dynamic crosshair states (?)

    // Crosshair Texture
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UTexture2D* Texture;

    // Crosshair Coloration
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FLinearColor Color;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FVector2D Scale;
};
