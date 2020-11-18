// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"

#include "UR_Pickup.h"

#include "UR_AmmoPickup.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AUR_Ammo;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
* Ammo Pickup Base Class
*/
UCLASS(Abstract, Blueprintable)
class OPENTOURNAMENT_API AUR_AmmoPickup : public AUR_Pickup
{
    GENERATED_BODY()
    
public:	

    AUR_AmmoPickup(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    /////////////////////////////////////////////////////////////////////////////////////////////////

    bool OnPickup_Implementation(AUR_Character* PickupCharacter) override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<AUR_Ammo> AmmoClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    int32 AmmoAmount;
};
