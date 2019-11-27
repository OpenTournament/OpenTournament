// Copyright (c) 2019 Open Tournament Project, All Rights Reserved.


#include "UR_WeaponBase.h"
#include "UR_Character.h"
#include "Engine/World.h"
#include "UR_Weapon.h"

AUR_WeaponBase::AUR_WeaponBase()
{
}

#if WITH_EDITOR
#include "Logging/MessageLog.h"
#include "Misc/UObjectToken.h"
void AUR_WeaponBase::CheckForErrors()
{
	Super::CheckForErrors();

	if (!HasAnyFlags(RF_ClassDefaultObject) && !IsValid(WeaponClass))
	{
		FMessageLog("MapCheck").Warning()
			->AddToken(FUObjectToken::Create(this))
			->AddToken(FTextToken::Create(FText::FromString(TEXT("has no valid weapon class"))));
	}
}
#endif

void AUR_WeaponBase::BeginPlay()
{
	// Avoid doing bullshit if we don't even have a valid weapon class
	if (!IsValid(WeaponClass))
	{
		bPickupAvailable = false;
		bRepInitialPickupAvailable = false;
		ShowPickupAvailable(false);
	}
	else
		Super::BeginPlay();
}

void AUR_WeaponBase::GiveTo_Implementation(class AActor* Other)
{
	AUR_Character* Char = Cast<AUR_Character>(Other);
	if (Char)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Owner = Char;
		SpawnParams.Instigator = Char;
		AUR_Weapon* Weap = GetWorld()->SpawnActor<AUR_Weapon>(WeaponClass, CapsuleComponent->GetComponentLocation(), FRotator(), SpawnParams);
		if (Weap)
		{
			Weap->PlayerController = Char;
			Weap->Pickup();
		}
	}
	Super::GiveTo_Implementation(Other);
}

FText AUR_WeaponBase::GetItemName_Implementation()
{
	if (WeaponClass)
		return FText::FromString(WeaponClass->GetDefaultObject<AUR_Weapon>()->WeaponName);

	return FText::FromString(TEXT("void"));
}

USkeletalMesh* AUR_WeaponBase::GetMeshForWeapon(const TSubclassOf<AUR_Weapon> InWeaponClass)
{
	if (InWeaponClass)
	{
		USkeletalMeshComponent* MeshComp = InWeaponClass->GetDefaultObject<AUR_Weapon>()->GetWeaponMesh();
		if (MeshComp)
			return MeshComp->SkeletalMesh;
	}
	return nullptr;
}
