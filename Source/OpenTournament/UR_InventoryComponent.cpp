// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_InventoryComponent.h"

#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

#include "OpenTournament.h"
#include "UR_Weapon.h"
#include "UR_Ammo.h"
#include "UR_Character.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_InventoryComponent::UUR_InventoryComponent()
{
    SetIsReplicatedByDefault(true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_InventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(UUR_InventoryComponent, InventoryW, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(UUR_InventoryComponent, ActiveWeapon, COND_OwnerOnly);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_InventoryComponent::Add(AUR_Weapon* InWeapon)
{
    if (InventoryW.Contains(InWeapon))
    {
        // If we already have this weapon instance... there is a logic error
        UE_LOG(LogTemp, Warning, TEXT("%s: weapon instance is already in inventory..."), *GetName());
        return;
    }

    // If we already have this weapon class, just stack ammo
    for (AUR_Weapon* IterWeapon : InventoryW)
    {
        if (IterWeapon->GetClass() == InWeapon->GetClass())
        {
            const int32 NewAmmoCount = IterWeapon->AmmoCount + InWeapon->AmmoCount;
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("%s ammo count %i -> %i"), *IterWeapon->WeaponName, IterWeapon->AmmoCount, NewAmmoCount));
            IterWeapon->AmmoCount = NewAmmoCount;
            InWeapon->Destroy();
            return;
        }
    }

    // Else, add weapon
    InventoryW.Add(InWeapon);
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("You have the %s (ammo = %i)"), *InWeapon->WeaponName, InWeapon->AmmoCount));

    // In standalone or listen host, call OnRep next tick so we can pick amongst new weapons what to swap to.
    if (Cast<ACharacter>(GetOwner()) && Cast<ACharacter>(GetOwner())->IsLocallyControlled())
    {
        GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UUR_InventoryComponent::OnRep_InventoryW);
    }
}

void UUR_InventoryComponent::Add(AUR_Ammo* InAmmo)
{
    /*if (InventoryA.Contains(ammo)) {
        for (auto& ammo2 : InventoryA)
        {
            if (ammo2->AmmoName == *ammo->AmmoName) {
                ammo2->amount += ammo->amount;
                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, FString::Printf(TEXT("Added %s ammo. Current amount: %d"), *ammo2->AmmoName, ammo2->amount));
            }
        }
    }
    else {*/
        //InventoryA.Add(ammo);
        //GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("You picked the %s"), *ammo->AmmoName));
    //}
        //UpdateWeaponAmmo(ammo);
}

void UUR_InventoryComponent::AmmoCountInInventory(AUR_Weapon* InWeapon) 
{
    for (auto& IterAmmo : InventoryA)
    {
        /*if (weapon->AmmoName == *ammo->AmmoName)
        {
            weapon->AmmoCount += ammo->amount;
        }*/
    }
}

void UUR_InventoryComponent::UpdateWeaponAmmo(AUR_Ammo* InAmmo) 
{
    for (auto& IterWeapon : InventoryW)
    {
        /*if (weapon->AmmoName == *ammo->AmmoName)
        {
            weapon->AmmoCount += ammo->amount;
        }*/
    }
}


void UUR_InventoryComponent::ShowInventory()
{
    for (auto& IterWeapon : InventoryW)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Weapons in inventory: %s with Ammo Count: %d"), *IterWeapon->WeaponName, IterWeapon->AmmoCount));
    }

    for (auto& IterAmmo : InventoryA)
    {
        //GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Ammo in inventory: %s"), *ammo->AmmoName));
    }

}

int32 UUR_InventoryComponent::SelectWeapon(int32 WeaponGroup)
{
    FString DesiredWeaponName = (TArray<FString> {
        TEXT("Assault Rifle"),
        TEXT("Shotgun"),
        TEXT("Rocket Launcher"),
        TEXT("Grenade Launcher"),
        TEXT("Sniper Rifle"),
        TEXT("Pistol"),
    })[WeaponGroup];

    for (auto& IterWeapon : InventoryW)
    {
        if (IterWeapon->WeaponName == DesiredWeaponName)
        {
            EquipWeapon(IterWeapon);
            return WeaponGroup;
        }
    }
    return 0;
}

AUR_Weapon * UUR_InventoryComponent::SelectWeaponG(int32 WeaponGroup)
{
    FString DesiredWeaponName = (TArray<FString> {
        TEXT("Assault Rifle"),
        TEXT("Shotgun"),
        TEXT("Rocket Launcher"),
        TEXT("Grenade Launcher"),
        TEXT("Sniper Rifle"),
        TEXT("Pistol"),
    })[WeaponGroup];

    for (auto& IterWeapon : InventoryW)
    {
        if (IterWeapon->WeaponName == DesiredWeaponName)
        {
            EquipWeapon(IterWeapon);
            break;
        }
    }
    return ActiveWeapon;
}

bool UUR_InventoryComponent::NextWeapon()
{
    AUR_Weapon* NewWeapon = nullptr;

    for (int32 i = 0; i < InventoryW.Num(); i++)
    {
        if (InventoryW[i] == ActiveWeapon)
            NewWeapon = InventoryW[(i + 1) % InventoryW.Num()];
    }
    if (!NewWeapon && InventoryW.Num() > 0)
        NewWeapon = InventoryW[0];

    if (NewWeapon && NewWeapon != ActiveWeapon)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Next weapon -> %s"), *NewWeapon->WeaponName));
        EquipWeapon(NewWeapon);
        return true;
    }
    return false;
}

bool UUR_InventoryComponent::PrevWeapon()
{
    AUR_Weapon* NewWeapon = nullptr;

    for (int32 i = 0; i < InventoryW.Num(); i++)
    {
        if (InventoryW[i] == ActiveWeapon)
            NewWeapon = InventoryW[(i + InventoryW.Num() - 1) % InventoryW.Num()];
    }
    if (!NewWeapon && InventoryW.Num() > 0)
        NewWeapon = InventoryW.Last();

    if (NewWeapon && NewWeapon != ActiveWeapon)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Prev weapon -> %s"), *NewWeapon->WeaponName));
        EquipWeapon(NewWeapon);
        return true;
    }
    return false;
}

void UUR_InventoryComponent::EquipWeapon(AUR_Weapon* InWeapon)
{
    if (ActiveWeapon)
    {
        ActiveWeapon->SetEquipped(false);
    }

    ServerEquipWeapon(InWeapon);

    InWeapon->SetEquipped(true);
    ActiveWeapon = InWeapon;
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Equipped: %s"), *InWeapon->WeaponName));
}

void UUR_InventoryComponent::ServerEquipWeapon_Implementation(AUR_Weapon* Weap)
{
    if (Weap && InventoryW.Contains(Weap))
    {
        if (ActiveWeapon)
            ActiveWeapon->SetEquipped(false);

        ActiveWeapon = Weap;
        ActiveWeapon->SetEquipped(true);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Client equipping invalid weapon: %s"), Weap ? *Weap->GetName() : TEXT("NULL"));
    }
}

void UUR_InventoryComponent::OnRep_InventoryW()
{
    if (!ActiveWeapon)
    {
        // This should only happen when we are given initial inventory on spawn
        // Here we should use user settings to pick the preferred weapon (if there are multiple).
        for (AUR_Weapon* IterWeapon : InventoryW)
        {
            if (IterWeapon)
            {
                EquipWeapon(IterWeapon);
                break;
            }
        }
    }
}

void UUR_InventoryComponent::OnRep_ActiveWeapon()
{
    // Here we can make sure server has the same equipped weapon as us.
    // If not, we might want to re-equip, or replace local equipped weapon...
}

void UUR_InventoryComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
    // !IsUnreachable() avoids crash during endgame world cleanup, trying to resolve bp-enabled events
    if (GetOwnerRole() == ROLE_Authority && !IsUnreachable())
    {
        //TODO: drop active weapon

        Clear();
    }

    Super::OnComponentDestroyed(bDestroyingHierarchy);
}

void UUR_InventoryComponent::Clear_Implementation()
{
    for (AUR_Weapon* IterWeapon : InventoryW)
    {
        if (IterWeapon)
            IterWeapon->Destroy();
    }
    InventoryW.Empty();

    for (AUR_Ammo* IterAmmo : InventoryA)
    {
        if (IterAmmo)
        {
            IterAmmo->Destroy();
        }
    }
    InventoryA.Empty();
}
