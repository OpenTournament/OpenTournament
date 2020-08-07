// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_AmmoPickup.h"

#include "OpenTournament.h"
#include "UR_Character.h"
#include "UR_InventoryComponent.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_AmmoPickup::AUR_AmmoPickup(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer),
    WeaponClass(nullptr),
    AmmoValue(10)
{
    DisplayName = FString("Ammo");
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_AmmoPickup::OnPickup_Implementation(AUR_Character* PickupCharacter)
{
    Super::OnPickup_Implementation(PickupCharacter);

    // @! TODO : Probably preferable to handle this via GameplayEffect in order to reduce custom logic in pickup class.
    if (PickupCharacter && WeaponClass)
    {
        if (const auto InventoryComponent = PickupCharacter->InventoryComponent)
        {
            for (auto& Weapon : InventoryComponent->WeaponArray)
            {
                if (WeaponClass == Weapon->GetClass())
                {
                    // @! TODO : Confusing/Bad Weapon-Ammo API here.
                    Weapon->ConsumeAmmo(-1 * AmmoValue);

                    // @! TODO : No AmmoMax value currently (and we probably shouldn't do this here. But stubbed just in case)
                    //if (Weapon->GetCurrentAmmo() > Weapon->AmmoMax)
                    //{
                    //    Weapon->AmmoCount = Weapon->AmmoMax;
                    //}
                }
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

