// Copyright 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Components/ActorComponent.h"

#include "OpenTournament.h"

#include "UR_HealthComponent.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declaration


/////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * HealthComponent is the base component for use by actors which can receive damage.
 */
UCLASS(DefaultToInstanced, BlueprintType, meta = (Tooltip = "A HealthComponent is a reusable component that can be added to any actor to give it a Health value.", ShortTooltip = "A HealthComponent is a reusable component that can be added to any actor to give it a Health value."), hideCategories = (UR, Character, Collision, Cooking))
class OPENTOURNAMENT_API UUR_HealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UUR_HealthComponent();

    /**
    * Health value.
    */
    UPROPERTY(Replicated, BlueprintReadOnly, EditDefaultsOnly, Category = "HealthComponent")
    int32 Health;

    /**
    * HealthMaximum value.
    */
    UPROPERTY(Replicated, BlueprintReadOnly, EditDefaultsOnly, Category = "HealthComponent")
    int32 HealthMax;

    /**
    * Set health to a numerical value
    */
    UFUNCTION(BlueprintCallable, Category = "HealthComponent")
    void SetHealth(const int32 InValue);

    /**
    * Set health to a percentage value
    */
    UFUNCTION(BlueprintCallable, Category = "HealthComponent")
    void SetHealthPercentage(const int32 InPercentage);

    /**
    * Change health by a value
    */
    UFUNCTION(BlueprintCallable, Category = "HealthComponent")
    void ChangeHealth(const int32 InChangeValue);

    /**
    * Change health by a percentage
    */
    UFUNCTION(BlueprintCallable, Category = "HealthComponent")
    void ChangeHealthPercentage(const int32 InChangePercentage);
};
