// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UR_Pickup_Dropped.h"

#include "UR_Pickup_DroppedWeapon.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AUR_Weapon;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
UCLASS(Blueprintable)
class OPENTOURNAMENT_API AUR_Pickup_DroppedWeapon
    : public AUR_Pickup_Dropped
{
    GENERATED_BODY()

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    virtual bool OnPickup_Implementation(AUR_Character* PickupCharacter) override;

    virtual void Destroyed() override;

public:
    AUR_Pickup_DroppedWeapon(const FObjectInitializer& ObjectInitializer);

    /**
    * Weapon instance held by this dropped pickup
    */
    UPROPERTY(ReplicatedUsing = OnRep_Weapon)
    AUR_Weapon* Weapon;

    UFUNCTION()
    virtual void SetWeapon(AUR_Weapon* InWeapon);

    UFUNCTION()
    virtual void OnRep_Weapon()
    {
        SetWeapon(Weapon);
    }
};
