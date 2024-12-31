// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Animation/AnimInstance.h"

#include "GameplayEffectTypes.h"

#include "UR_AnimInstance.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UAbilitySystemComponent;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * UUR_AnimInstance
 *
 *	The base game animation instance class used by this project.
 */
UCLASS(Config = Game)
class UUR_AnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UUR_AnimInstance(const FObjectInitializer& ObjectInitializer);

    virtual void InitializeWithAbilitySystem(UAbilitySystemComponent* ASC);

protected:
#if WITH_EDITOR
    virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif // WITH_EDITOR

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
    // Gameplay tags that can be mapped to blueprint variables. The variables will automatically update as the tags are added or removed.
    // These should be used instead of manually querying for the gameplay tags.
    UPROPERTY(EditDefaultsOnly, Category = "GameplayTags")
    FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap;

    UPROPERTY(BlueprintReadOnly, Category = "Character State Data")
    float GroundDistance = -1.0f;
};
