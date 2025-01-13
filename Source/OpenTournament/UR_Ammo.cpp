// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Ammo.h"

#include "Net/UnrealNetwork.h"

#include "UR_Character.h"
#include "UR_InventoryComponent.h"
#include "UR_Weapon.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_Ammo)

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_Ammo::AUR_Ammo()
{
    PrimaryActorTick.bCanEverTick = false;

    bReplicates = true;
    bOnlyRelevantToOwner = true;
    SetReplicatingMovement(false);

    AmmoName = FText::FromString(TEXT("Ammo"));
    MaxAmmo = 30;
    AmmoCount = 0;
}

void AUR_Ammo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(ThisClass, AmmoCount, COND_OwnerOnly);
}

void AUR_Ammo::StackAmmo(int32 InAmount, AUR_Weapon* FromWeapon)
{
    int32 AmmoCap = MaxAmmo;

    //NOTE: Here we can add something to handle quake-like case, to prevent weapon ammo stacking.
    // I don't want to use properties for this because it's probably gonna end up depending on gamemode.
    // For example casual FFA would have fast respawn times ==> prevent weapon ammo stacking
    // But Duel have long respawn times ==> allow weapon ammo stacking
    // So the gamemode should be responsible for controlling this behavior.

    // We can add a hook here which does something like this :
    /*
    if (InventoryComponent->WeaponArray.Contains(FromWeapon))
    {
        int32 WeaponAmmo = 0;
        for (const auto& AmmoDef : FromWeapon->AmmoDefinitions)
        {
            if (AmmoDef.AmmoClass == StaticClass() && AmmoDef.AmmoAmount > WeaponAmmo)
            {
                WeaponAmmo = AmmoDef.AmmoAmount;
            }
        }
        if (WeaponAmmo > 0)
        {
            AmmoCap = WeaponAmmo + 1;
        }
    }
    */

    if (AmmoCount < AmmoCap)
    {
        SetAmmoCount(FMath::Min(AmmoCount + InAmount, AmmoCap));
    }
}

void AUR_Ammo::ConsumeAmmo(int32 Amount)
{
    SetAmmoCount(AmmoCount - Amount);
}

void AUR_Ammo::SetAmmoCount(int32 NewAmmoCount)
{
    NewAmmoCount = FMath::Clamp(NewAmmoCount, 0, 999);
    if (NewAmmoCount != AmmoCount)
    {
        int32 OldAmmoCount = AmmoCount;
        AmmoCount = NewAmmoCount;
        OnRep_AmmoCount(OldAmmoCount);
    }
}

void AUR_Ammo::OnRep_AmmoCount(int32 OldAmmoCount)
{
    if (AmmoCount != OldAmmoCount)
    {
        if (auto Char = Cast<AUR_Character>(GetOwner()); IsValid(Char))
        {
            if (auto InventoryComponent = Char->GetInventoryComponent(); IsValid(InventoryComponent))
            {
                InventoryComponent->OnAmmoUpdated.Broadcast(Char->InventoryComponent, this);

                if (auto ActiveWeapon = InventoryComponent->GetActiveWeapon(); IsValid(ActiveWeapon))
                {
                    if (ActiveWeapon->AmmoRefs.Contains(this))
                    {
                        Char->InventoryComponent->ActiveWeapon->NotifyAmmoUpdated(this);
                    }
                }

            }
        }
    }
}
