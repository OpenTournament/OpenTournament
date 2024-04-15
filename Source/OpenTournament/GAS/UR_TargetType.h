// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Abilities/GameplayAbilityTypes.h"

#include "UR_AbilityTypes.h"

#include "UR_TargetType.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AUR_Character;
class AActor;
struct FGameplayEventData;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Class that is used to determine targeting for abilities
 * It is meant to be blueprinted to run target logic
 * This does not subclass GameplayAbilityTargetActor because this class is never instanced into the world
 * This can be used as a basis for a game-specific targeting blueprint
 * If your targeting is more complicated you may need to instance into the world once or as a pooled actor
 */
UCLASS(Blueprintable, meta = (ShowWorldContextPin))
class OPENTOURNAMENT_API UUR_TargetType : public UObject
{
    GENERATED_BODY()

public:
    // Constructor and overrides
    UUR_TargetType()
    {
    }

    /** Called to determine targets to apply gameplay effects to */
    UFUNCTION(BlueprintNativeEvent)
    void GetTargets(AUR_Character* TargetingCharacter, AActor* TargetingActor, FGameplayEventData EventData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
* Trivial target type that uses the owner
*/
UCLASS(NotBlueprintable)
class OPENTOURNAMENT_API UUR_TargetType_UseOwner : public UUR_TargetType
{
    GENERATED_BODY()

public:
    // Constructor and overrides
    UUR_TargetType_UseOwner()
    {
    }

    /** Uses the passed in event data */
    virtual void GetTargets_Implementation(AUR_Character* TargetingCharacter, AActor* TargetingActor, FGameplayEventData EventData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const override;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
* Trivial target type that pulls the target out of the event data
*/
UCLASS(NotBlueprintable)
class OPENTOURNAMENT_API UUR_TargetType_UseEventData : public UUR_TargetType
{
    GENERATED_BODY()

public:
    // Constructor and overrides
    UUR_TargetType_UseEventData()
    {
    }

    /** Uses the passed in event data */
    virtual void GetTargets_Implementation(AUR_Character* TargetingCharacter, AActor* TargetingActor, FGameplayEventData EventData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const override;
};
