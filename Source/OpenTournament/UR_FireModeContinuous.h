// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UR_FireModeBase.h"
#include "UR_FireModeContinuous.generated.h"


/**
* Event dispatcher.
* Called every tick during continuous firing.
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFiringTickSignature, UUR_FireModeContinuous*, FireMode);


/**
 * 
 */
UCLASS(ClassGroup = (FireMode), Meta = (BlueprintSpawnableComponent))
class OPENTOURNAMENT_API UUR_FireModeContinuous : public UUR_FireModeBase
{
	GENERATED_BODY()

public:

    UPROPERTY(BlueprintAssignable)
    FFiringTickSignature OnFiringTick;

};
