// Copyright 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_InventoryComponent.h"

#include "UnrealNetwork.h"
#include "Engine.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_InventoryComponent::UUR_InventoryComponent()
{
    SetIsReplicatedByDefault(true);
}

void UUR_InventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(UUR_InventoryComponent, InventoryW, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(UUR_InventoryComponent, ActiveWeapon, COND_OwnerOnly);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_InventoryComponent::Add(AUR_Weapon* weapon)
{
    if (InventoryW.Contains(weapon))
    {
        // If we already have this weapon instance... there is a logic error
        UE_LOG(LogTemp, Warning, TEXT("%s: weapon instance is already in inventory..."), *GetName());
        return;
    }

    // If we already have this weapon class, just stack ammo
    for (AUR_Weapon* Weap : InventoryW)
    {
        if (Weap->GetClass() == weapon->GetClass())
        {
            int32 NewAmmoCount = Weap->ammoCount + weapon->ammoCount;
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("%s ammo count %i -> %i"), *Weap->WeaponName, Weap->ammoCount, NewAmmoCount));
            Weap->ammoCount = NewAmmoCount;
            weapon->Destroy();
            return;
        }
    }

    // Else, add weapon
    InventoryW.Add(weapon);
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("You have the %s (ammo = %i)"), *weapon->WeaponName, weapon->ammoCount));

    // In standalone or listen host, call OnRep next tick so we can pick amongst new weapons what to swap to.
    if (Cast<ACharacter>(GetOwner()) && Cast<ACharacter>(GetOwner())->IsLocallyControlled())
    {
        GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UUR_InventoryComponent::OnRep_InventoryW);
    }
}

void UUR_InventoryComponent::Add(AUR_Ammo* ammo)
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
        InventoryA.Add(ammo);
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("You picked the %s"), *ammo->AmmoName));
    //}
        UpdateWeaponAmmo(ammo);
}

void UUR_InventoryComponent::AmmoCountInInventory(AUR_Weapon* weapon) 
{
    for (auto& ammo : InventoryA)
    {
        if (weapon->AmmoName == *ammo->AmmoName) {
            weapon->ammoCount += ammo->amount;
        }
    }
}

void UUR_InventoryComponent::UpdateWeaponAmmo(AUR_Ammo* ammo) 
{
    for (auto& weapon : InventoryW)
    {
        if (weapon->AmmoName == *ammo->AmmoName) {
            weapon->ammoCount += ammo->amount;
        }
    }
}


void UUR_InventoryComponent::ShowInventory()
{
    for (auto& weapon : InventoryW)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Weapons in inventory: %s with Ammo Count: %d"), *weapon->WeaponName, weapon->ammoCount));
    }

    for (auto& ammo : InventoryA)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Ammo in inventory: %s"), *ammo->AmmoName));
    }

}

int32 UUR_InventoryComponent::SelectWeapon(int32 number)
{
    FString DesiredWeaponName = (TArray<FString> {
        TEXT("Assault Rifle"),
        TEXT("Shotgun"),
        TEXT("Rocket Launcher"),
        TEXT("Grenade Launcher"),
        TEXT("Sniper Rifle"),
        TEXT("Pistol"),
    })[number];

    for (auto& weapon : InventoryW)
    {
        if (weapon->WeaponName == DesiredWeaponName)
        {
            EquipWeapon(weapon);
            return number;
        }
    }
    return 0;
}

AUR_Weapon * UUR_InventoryComponent::SelectWeaponG(int32 number)
{
    FString DesiredWeaponName = (TArray<FString> {
        TEXT("Assault Rifle"),
        TEXT("Shotgun"),
        TEXT("Rocket Launcher"),
        TEXT("Grenade Launcher"),
        TEXT("Sniper Rifle"),
        TEXT("Pistol"),
    })[number];

    for (auto& weapon : InventoryW)
    {
        if (weapon->WeaponName == DesiredWeaponName)
        {
            EquipWeapon(weapon);
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

void UUR_InventoryComponent::EquipWeapon(AUR_Weapon* Weap)
{
    if (ActiveWeapon)
    {
        ActiveWeapon->setEquipped(false);
    }

    ServerEquipWeapon(Weap);

    Weap->setEquipped(true);
    ActiveWeapon = Weap;
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Equipped: %s"), *Weap->WeaponName));
}

void UUR_InventoryComponent::ServerEquipWeapon_Implementation(AUR_Weapon* Weap)
{
    if (Weap && InventoryW.Contains(Weap))
    {
        if (ActiveWeapon)
            ActiveWeapon->setEquipped(false);

        ActiveWeapon = Weap;
        ActiveWeapon->setEquipped(true);
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
        for (AUR_Weapon* Weap : InventoryW)
        {
            if (Weap)
            {
                EquipWeapon(Weap);
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
    if (GetOwnerRole() == ROLE_Authority)
    {
        //TODO: drop active weapon

        for (AUR_Weapon* Weap : InventoryW)
        {
            if (Weap)
                Weap->Destroy();
        }
        InventoryW.Empty();

        for (AUR_Ammo* Ammo : InventoryA)
        {
            if (Ammo)
                Ammo->Destroy();
        }
        InventoryA.Empty();
    }

    Super::OnComponentDestroyed(bDestroyingHierarchy);
}