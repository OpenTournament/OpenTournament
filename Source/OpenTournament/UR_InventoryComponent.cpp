// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_InventoryComponent.h"

#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Engine.h"

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

    DOREPLIFETIME_CONDITION(UUR_InventoryComponent, WeaponArray, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(UUR_InventoryComponent, AmmoArray, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(UUR_InventoryComponent, DesiredWeapon, COND_SkipOwner);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

bool UUR_InventoryComponent::IsLocallyControlled() const
{
    APawn* P = Cast<APawn>(GetOwner());
    return P && P->IsLocallyControlled();
}

void UUR_InventoryComponent::AddWeapon(AUR_Weapon* InWeapon)
{
    if (WeaponArray.Contains(InWeapon))
    {
        // If we already have this weapon instance... there is a logic error
        UE_LOG(LogTemp, Warning, TEXT("%s: weapon instance is already in inventory..."), *GetName());
        return;
    }

    // Initialize or stack ammo
    TSet<TSubclassOf<AUR_Ammo>> WeaponAmmoClasses(InWeapon->AmmoClasses);
    for (const auto& AmmoClass : WeaponAmmoClasses)
    {
        GetAmmoByClass(AmmoClass, true)->StackWeapon();
    }

    // If we already have this weapon class, do nothing more
    for (AUR_Weapon* Weap : WeaponArray)
    {
        if (Weap->GetClass() == InWeapon->GetClass())
        {
            InWeapon->Destroy();
            return;
        }
    }

    // Else, add weapon
    WeaponArray.Add(InWeapon);
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("You have the %s (ammo = %i)"), *InWeapon->WeaponName, InWeapon->GetCurrentAmmo()));

    // Set ammo refs
    InWeapon->AmmoRefs.SetNumZeroed(InWeapon->AmmoClasses.Num());
    for (int32 i = 0; i < InWeapon->AmmoClasses.Num(); i++)
    {
        InWeapon->AmmoRefs[i] = GetAmmoByClass(InWeapon->AmmoClasses[i]);
    }

    // In standalone or listen host, call OnRep next tick so we can pick amongst new weapons what to swap to.
    if (IsLocallyControlled())
    {
        GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UUR_InventoryComponent::OnRep_WeaponArray);
    }
}

void UUR_InventoryComponent::AddAmmo(TSubclassOf<AUR_Ammo> InAmmoClass)
{
    GetAmmoByClass(InAmmoClass, true)->StackAmmoPack();
}

AUR_Ammo* UUR_InventoryComponent::GetAmmoByClass(TSubclassOf<AUR_Ammo> InAmmoClass, bool bAllowCreate)
{
    if (InAmmoClass)
    {
        for (AUR_Ammo* Ammo : AmmoArray)
        {
            if (Ammo->GetClass() == InAmmoClass)
            {
                return Ammo;
            }
        }

        if (bAllowCreate && GetOwnerRole() == ROLE_Authority)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            SpawnParams.Owner = GetOwner();
            AUR_Ammo* NewAmmo = GetWorld()->SpawnActor<AUR_Ammo>(InAmmoClass, SpawnParams);
            if (NewAmmo)
            {
                AmmoArray.Add(NewAmmo);
                return NewAmmo;
            }
        }
    }

    return NULL;
}

void UUR_InventoryComponent::ShowInventory()
{
    for (auto& IterWeapon : WeaponArray)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Weapons in inventory: %s with Ammo Count: %d"), *IterWeapon->WeaponName, IterWeapon->GetCurrentAmmo()));
    }
    for (auto& IterAmmo : AmmoArray)
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

    for (auto& IterWeapon : WeaponArray)
    {
        if (IterWeapon->WeaponName == DesiredWeaponName)
        {
            SetDesiredWeapon(IterWeapon);
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

    for (auto& IterWeapon : WeaponArray)
    {
        if (IterWeapon->WeaponName == DesiredWeaponName)
        {
            SetDesiredWeapon(IterWeapon);
            break;
        }
    }
    return ActiveWeapon;
}

bool UUR_InventoryComponent::NextWeapon()
{
    if (WeaponArray.Num() == 0)
    {
        return false;
    }

    AUR_Weapon* NewWeapon = nullptr;

    int32 CurrentIndex;
    WeaponArray.Find(DesiredWeapon, CurrentIndex);

    for (int32 i = (CurrentIndex + 1) % WeaponArray.Num(); i != CurrentIndex; i = (i + 1) % WeaponArray.Num())
    {
        if (WeaponArray[i] && WeaponArray[i]->HasAnyAmmo())
        {
            NewWeapon = WeaponArray[i];
            break;
        }
    }

    if (NewWeapon && NewWeapon != DesiredWeapon)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Next weapon -> %s"), *NewWeapon->WeaponName));
        SetDesiredWeapon(NewWeapon);
        return true;
    }
    return false;
}

bool UUR_InventoryComponent::PrevWeapon()
{
    if (WeaponArray.Num() == 0)
    {
        return false;
    }

    AUR_Weapon* NewWeapon = nullptr;

    int32 CurrentIndex;
    WeaponArray.Find(DesiredWeapon, CurrentIndex);
    CurrentIndex = FMath::Max(0, CurrentIndex);

    for (int32 i = (WeaponArray.Num() + CurrentIndex - 1) % WeaponArray.Num(); i != CurrentIndex; i = (WeaponArray.Num() + i - 1) % WeaponArray.Num())
    {
        if (WeaponArray[i] && WeaponArray[i]->HasAnyAmmo())
        {
            NewWeapon = WeaponArray[i];
            break;
        }
    }

    if (NewWeapon && NewWeapon != DesiredWeapon)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Prev weapon -> %s"), *NewWeapon->WeaponName));
        SetDesiredWeapon(NewWeapon);
        return true;
    }
    return false;
}

void UUR_InventoryComponent::SetDesiredWeapon(AUR_Weapon* InWeapon)
{
    DesiredWeapon = InWeapon;

    if (ActiveWeapon && ActiveWeapon->WeaponState != EWeaponState::Inactive)
    {
        if ( ActiveWeapon != DesiredWeapon )
        {
            ActiveWeapon->RequestPutDown();
        }
        else
        {
            ActiveWeapon->RequestBringUp();
        }
    }
    else
    {
        SetActiveWeapon(DesiredWeapon);
    }

    if (GetNetMode() == NM_Client && IsLocallyControlled())
    {
        ServerSetDesiredWeapon(InWeapon);
    }
}

void UUR_InventoryComponent::ServerSetDesiredWeapon_Implementation(AUR_Weapon* InWeapon)
{
    // On server
    SetDesiredWeapon(InWeapon);
}

void UUR_InventoryComponent::OnRep_DesiredWeapon()
{
    // On remote clients
    SetDesiredWeapon(DesiredWeapon);
}

void UUR_InventoryComponent::OnActiveWeaponStateChanged(AUR_Weapon* Weapon, EWeaponState NewState)
{
    if (Weapon && Weapon == ActiveWeapon && NewState == EWeaponState::Inactive)
    {
        SetActiveWeapon(DesiredWeapon);
    }
}

void UUR_InventoryComponent::SetActiveWeapon(AUR_Weapon* InWeapon)
{
    if (ActiveWeapon)
    {
        ActiveWeapon->OnWeaponStateChanged.RemoveDynamic(this, &UUR_InventoryComponent::OnActiveWeaponStateChanged);

        // Edge case - eg. weapondrop doesn't go through the putdown procedure.
        if (ActiveWeapon->WeaponState != EWeaponState::Inactive)
        {
            // this ensures the mesh is detached from pawn
            ActiveWeapon->SetWeaponState(EWeaponState::Inactive);
        }

        ActiveWeapon = NULL;
    }

    //NOTE: Weapons array is not replicated for non-owning clients
    if (InWeapon && (GetNetMode() == NM_Client || WeaponArray.Contains(InWeapon)))
    {
        ActiveWeapon = InWeapon;
        ActiveWeapon->RequestBringUp();
        ActiveWeapon->OnWeaponStateChanged.AddUniqueDynamic(this, &UUR_InventoryComponent::OnActiveWeaponStateChanged);
    }

    //modsupport - case where SetActiveWeapon is called without setting DesiredWeapon.
    // We need to replicate to remote clients via DesiredWeapon.
    if (GetNetMode() != NM_Client)
    {
        DesiredWeapon = ActiveWeapon;
    }
}

void UUR_InventoryComponent::OnRep_WeaponArray()
{
    if (!ActiveWeapon)
    {
        // This should only happen when we are given initial inventory on spawn
        // Here we should use user settings to pick the preferred weapon (if there are multiple).
        for (AUR_Weapon* IterWeapon : WeaponArray)
        {
            if (IterWeapon)
            {
                SetDesiredWeapon(IterWeapon);
                break;
            }
        }
    }
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
    for (AUR_Weapon* IterWeapon : WeaponArray)
    {
        if (IterWeapon)
            IterWeapon->Destroy();
    }
    WeaponArray.Empty();

    for (AUR_Ammo* IterAmmo : AmmoArray)
    {
        if (IterAmmo)
        {
            IterAmmo->Destroy();
        }
    }
    AmmoArray.Empty();
}
