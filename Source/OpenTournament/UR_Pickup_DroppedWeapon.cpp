// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Pickup_DroppedWeapon.h"

#include "Net/UnrealNetwork.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"

#include "UR_Weapon.h"
#include "UR_Character.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_Pickup_DroppedWeapon::AUR_Pickup_DroppedWeapon(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    CollisionComponent->SetCapsuleSize(30.f, 30.f, true);
}

void AUR_Pickup_DroppedWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(AUR_Pickup_DroppedWeapon, Weapon, COND_InitialOnly);
}

void AUR_Pickup_DroppedWeapon::SetWeapon(AUR_Weapon* InWeapon)
{
    Weapon = InWeapon;

    if (Weapon)
    {
        Weapon->SetWeaponState(EWeaponState::Dropped);

        Weapon->GetMesh3P()->AttachToComponent(StaticMesh, FAttachmentTransformRules::SnapToTargetIncludingScale);
        Weapon->GetMesh3P()->SetWorldRotation(FRotator(0.f, Weapon->GetMesh3P()->GetComponentRotation().Yaw, 0.f));
        Weapon->ToggleGeneralVisibility(true);

        DisplayName = FText::FromString(Weapon->WeaponName);
    }
}

bool AUR_Pickup_DroppedWeapon::OnPickup_Implementation(AUR_Character* PickupCharacter)
{
    if (Weapon && PickupCharacter && PickupCharacter->InventoryComponent)
    {
        Weapon->ToggleGeneralVisibility(false); //re-hide until attachment functions are called

        if (HasAuthority())
        {
            Weapon->GiveTo(PickupCharacter);
        }

        Weapon = NULL;  // don't destroy!
        return true;
    }
    return false;
}

void AUR_Pickup_DroppedWeapon::Destroyed()
{
    if (Weapon)
    {
        Weapon->Destroy();
        Weapon = NULL;
    }

    Super::Destroyed();
}
