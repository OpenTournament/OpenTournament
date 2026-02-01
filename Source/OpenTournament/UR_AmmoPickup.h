// Copyright (c) Open Tournament Games, All Rights Reserved.

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

    virtual void CheckForErrors() override;
#endif

    /////////////////////////////////////////////////////////////////////////////////////////////////

    virtual bool OnPickup_Implementation(AUR_Character* PickupCharacter) override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<AUR_Ammo> AmmoClass_Internal;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<AUR_Ammo> AmmoClass_Soft;


    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    int32 AmmoAmount;
};
