// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_InventoryComponent.h"

#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Engine.h"

#include "OpenTournament.h"
#include "UR_Weapon.h"
#include "UR_Ammo.h"
#include "UR_UserSettings.h"

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
    for (const auto& AmmoDef : InWeapon->AmmoDefinitions)
    {
        if (AmmoDef.AmmoClass)
        {
            GetAmmoByClass(AmmoDef.AmmoClass, true)->StackAmmo(AmmoDef.AmmoAmount, InWeapon);
        }
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

    // Message (temporary)
    if (auto Pawn = Cast<APawn>(GetOwner()))
    {
        if (auto PC = Pawn->GetController<APlayerController>())
        {
            PC->ClientMessage(FString::Printf(TEXT("You have the %s"), *InWeapon->WeaponName));
        }
    }

    // Set ammo refs
    InWeapon->AmmoRefs.SetNumZeroed(InWeapon->AmmoDefinitions.Num());
    for (int32 i = 0; i < InWeapon->AmmoDefinitions.Num(); i++)
    {
        InWeapon->AmmoRefs[i] = GetAmmoByClass(InWeapon->AmmoDefinitions[i].AmmoClass);
    }

    // In standalone or listen host, call OnRep next tick so we can pick amongst new weapons what to swap to.
    if (IsLocallyControlled())
    {
        GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UUR_InventoryComponent::OnRep_WeaponArray);
    }
}

void UUR_InventoryComponent::AddAmmo(TSubclassOf<AUR_Ammo> InAmmoClass, int32 InAmount)
{
    if (AUR_Ammo* Ammo = GetAmmoByClass(InAmmoClass, true))
    {
        Ammo->StackAmmo(InAmount);
    }
}

AUR_Ammo* UUR_InventoryComponent::GetAmmoByClass(TSubclassOf<AUR_Ammo> InAmmoClass, bool bAutoCreate)
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

        if (bAutoCreate && GetOwnerRole() == ROLE_Authority)
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

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_InventoryComponent::SelectWeapon(int32 Index)
{
    if (WeaponGroups.IsValidIndex(Index))
    {
        auto GroupWeapons = WeaponGroups[Index].Weapons.FilterByPredicate([&](AUR_Weapon* W) {
            return W && (W->HasAnyAmmo() || W == DesiredWeapon);
        });
        if (GroupWeapons.Num() > 0)
        {
            int32 SelectedIndex = GroupWeapons.Find(DesiredWeapon);
            int32 DesiredIndex = (SelectedIndex + 1) % GroupWeapons.Num();
            if (DesiredIndex != SelectedIndex)
            {
                SetDesiredWeapon(GroupWeapons[DesiredIndex]);
            }
        }
    }
}

bool UUR_InventoryComponent::NextWeapon()
{
    //NOTE: We want prev/next to match the visible weapon bar, because that is what makes the most sense.

    //NOTE: A weapon can be present in multiple groups (including in the visible weapon bar).
    // Handling prev/next weapon can be difficult.

    // Example: sniper is present in groups 1, 3 and 9.
    // Groups 1 and 3 are visible on weapon bar, group 9 is a hidden group with a keybind.
    // User selects sniper by pressing 9.
    // Groups 1 and 3 appear selected on bar because sniper is in both.
    // What should be the next weapon ?

    // Option 1 : prevent weapon from being present in multiple _visible_ groups.

    // Option 2 : build an array of weapons from visible groups, remove duplicates, then work with that.

    TArray<AUR_Weapon*> WorkArray;
    for (auto& Group : WeaponGroups)
    {
        if (Group.Visibility != EWeaponGroupVisibility::Hidden)
        {
            for (auto Weapon : Group.Weapons)
            {
                // Only include weappons with ammo, and also current weapon for reference
                if (Weapon && (Weapon->HasAnyAmmo() || Weapon == DesiredWeapon))
                {
                    WorkArray.AddUnique(Weapon);
                }
            }
        }
    }
    if (WorkArray.Num() > 0)
    {
        int32 SelectedIndex = WorkArray.Find(DesiredWeapon);
        int32 DesiredIndex = (SelectedIndex + 1) % WorkArray.Num();
        if (DesiredIndex != SelectedIndex)
        {
            SetDesiredWeapon(WorkArray[DesiredIndex]);
            return true;
        }
    }
    return false;
}

bool UUR_InventoryComponent::PrevWeapon()
{
    TArray<AUR_Weapon*> WorkArray;
    for (const auto& Group : WeaponGroups)
    {
        if (Group.Visibility != EWeaponGroupVisibility::Hidden)
        {
            for (auto Weapon : Group.Weapons)
            {
                if (Weapon && (Weapon->HasAnyAmmo() || Weapon == DesiredWeapon))
                {
                    WorkArray.AddUnique(Weapon);
                }
            }
        }
    }
    if (WorkArray.Num() > 0)
    {
        int32 SelectedIndex = WorkArray.Find(DesiredWeapon);
        int32 DesiredIndex = (WorkArray.Num() + (SelectedIndex != INDEX_NONE ? SelectedIndex : 0) - 1) % WorkArray.Num();
        if (DesiredIndex != SelectedIndex)
        {
            SetDesiredWeapon(WorkArray[DesiredIndex]);
            return true;
        }
    }
    return false;
}

void UUR_InventoryComponent::SetDesiredWeapon(AUR_Weapon* InWeapon)
{
    if (DesiredWeapon == InWeapon)
    {
        return;
    }

    AUR_Weapon* OldDesired = DesiredWeapon;
    DesiredWeapon = InWeapon;

    OnDesiredWeaponChanged.Broadcast(this, DesiredWeapon, OldDesired);

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

void UUR_InventoryComponent::OnRep_DesiredWeapon(AUR_Weapon* OldDesired)
{
    // On remote clients
    AUR_Weapon* NewDesired = DesiredWeapon;
    DesiredWeapon = OldDesired;
    SetDesiredWeapon(NewDesired);
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
    AUR_Weapon* OldActive = ActiveWeapon;

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

    OnActiveWeaponChanged.Broadcast(this, ActiveWeapon, OldActive);

    //modsupport - case where SetActiveWeapon is called without setting DesiredWeapon.
    // We need to replicate to remote clients via DesiredWeapon.
    if (GetNetMode() != NM_Client)
    {
        DesiredWeapon = ActiveWeapon;
    }
}

void UUR_InventoryComponent::OnRep_WeaponArray()
{
    RefillWeaponGroups();

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

void UUR_InventoryComponent::RefillWeaponGroups()
{
    if (auto Settings = UUR_UserSettings::Get(this))
    {
        WeaponGroups = Settings->WeaponGroups;
    }
    else
    {
        WeaponGroups.Empty();
        return;
    }

    // Need an array of array for each group.
    // Because one weapon class in a group can serve as a match for multiple weapon instances, due to fallbacks.
    // We want to keep the order of grouped weapons, by the order of matched weapon classes.
    // 1st index weapon group, 2nd index weapon class
    TArray<TArray<TArray<AUR_Weapon*>>> Temp;
    Temp.SetNumZeroed(WeaponGroups.Num());

    for (auto Weapon : WeaponArray)
    {
        if (!Weapon)    // object not replicated yet
            continue;

        bool bFound = false;
        for (UClass* TestClass = Weapon->GetClass(); !bFound && TestClass; TestClass = AUR_Weapon::GetNextFallbackConfigWeapon(TestClass))
        {
            for (int32 GroupIndex = 0; GroupIndex < WeaponGroups.Num(); GroupIndex++)
            {
                int32 ClassIndex = WeaponGroups[GroupIndex].WeaponClasses.Find(TestClass);
                if (ClassIndex != INDEX_NONE)
                {
                    if (Temp[GroupIndex].Num() <= ClassIndex)
                    {
                        Temp[GroupIndex].SetNumZeroed(ClassIndex + 1);
                    }
                    if (TestClass == Weapon->GetClass())
                    {
                        // In case of immediate match, insert at first position always
                        Temp[GroupIndex][ClassIndex].Insert(Weapon, 0);
                    }
                    else
                    {
                        // In case of fallback, append. If multiple weapons match by the same fallback, they will be in pickup order.
                        Temp[GroupIndex][ClassIndex].Add(Weapon);
                    }
                    bFound = true;
                }
            }
        }
    }

    // Concatenate arrays
    for (int32 GroupIndex = 0; GroupIndex < WeaponGroups.Num(); GroupIndex++)
    {
        WeaponGroups[GroupIndex].Weapons.Empty();
        for (auto& Weapons : Temp[GroupIndex])
        {
            WeaponGroups[GroupIndex].Weapons.Append(Weapons);
        }
    }

    OnWeaponGroupsUpdated.Broadcast(this);
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
