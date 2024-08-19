// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "UR_PaniniUtils.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations

class UMaterialInterface;
class UInterface;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
UCLASS()
class OPENTOURNAMENT_API UUR_PaniniUtils : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
    * Conventional name of the parameter that enables panini (MF_FirstPersonHandler)
    * We cannot change static switch parameters at runtime so we'll have to use scalar.
    */
    UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "Material|Panini")
    static FORCEINLINE FName PaniniParameterName()
    {
        return FName(TEXT("sPanini"));
    }

    /**
    * Toggles panini switch to all materials on this component.
    * - Works with mesh components
    * - Doesn't work with widget components - see notes in .cpp - could get fixed in the future
    * - Doesn't work with particle systems / dunno how to do it
    *
    * Mostly intended for weapons so we can stop duplicating material instances for their 1p variant.
    * In 95% of cases, they're the exact same material and just need panini enabled.
    */
    UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Material|Panini")
    static void TogglePaniniProjection(USceneComponent* Component, bool bEnablePanini = true, bool bPropagateToChildren = true);

    /**
    * Calculates panini projection offset for a single point in space.
    * Used to attach weapon effects (muzzle flash, beam starts) to the proper location.
    * This must be kept in sync with our MF_FirstPersonHandler material function.
    * This is obviously not ideal, not sure what the best course of action would be ???
    * Adding panini to all those effects could work, but it's a HUGE PAIN ???
    */
    UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "Material|Panini", Meta = (WorldContext = "WorldContext"))
    static FVector CalcPaniniProjection(const UObject* WorldContext, const FVector& WorldPos);

    /**
    * Calculates world position offset applied by MF_ShrinkWeapon (sub-function of our panini function)
    */
    UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "Material|Panini", Meta = (WorldContext = "WorldContext"))
    static FVector CalcMFShrinkWeapon(const UObject* WorldContext, const FVector& WorldPos);
};
