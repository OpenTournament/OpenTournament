// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include <Kismet/BlueprintFunctionLibrary.h>
#include "UR_GameplayStatics.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AController;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
UCLASS()
class OPENTOURNAMENT_API UUR_GameplayStatics : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:

    /** Hurts the specified actor with the specified impact.
     * @param DamagedActor - Actor that will be damaged.
     * @param BaseDamage - The base damage to apply.
     * @param HitFromDirection - Direction the hit came FROM
     * @param HitInfo - Collision or trace result that describes the hit
     * @param EventInstigator - Controller that was responsible for causing this damage (e.g. player who shot the weapon)
     * @param DamageCauser - Actor that actually caused the damage (e.g. the grenade that exploded)
     * @param DamageTypeClass - Class that describes the damage that was done.
     * @return Actual damage the ended up being applied to the actor.
     */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Game|Damage")
    static float ApplyGASPointDamage(AActor* DamagedActor, float BaseDamage, const FVector& HitFromDirection, const FHitResult& HitInfo, AController* EventInstigator, AActor* DamageCauser, TSubclassOf<class UDamageType> DamageTypeClass);

};
