// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_AmmoPickup.h"

#include "OpenTournament.h"
#include "UR_Character.h"
#include "UR_InventoryComponent.h"
#include "UR_Ammo.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_AmmoPickup::AUR_AmmoPickup(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer),
    AmmoClass(AUR_Ammo::StaticClass()),
    AmmoAmount(10)
{
    DisplayName = TEXT("Ammo");
}

#if WITH_EDITOR
void AUR_AmmoPickup::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (PropertyChangedEvent.GetPropertyName().IsEqual(FName(TEXT("AmmoClass"))))
    {
        DisplayName = AmmoClass ? AmmoClass->GetDefaultObject<AUR_Ammo>()->AmmoName : TEXT("Ammo");
    }
}
#endif

bool AUR_AmmoPickup::OnPickup_Implementation(AUR_Character* PickupCharacter)
{
    if (AmmoClass && PickupCharacter && PickupCharacter->InventoryComponent)
    {
        PickupCharacter->InventoryComponent->AddAmmo(AmmoClass, AmmoAmount);
    }

    return Super::OnPickup_Implementation(PickupCharacter);
}

/////////////////////////////////////////////////////////////////////////////////////////////////