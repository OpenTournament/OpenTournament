// Copyright (c) 2019 Open Tournament Project, All Rights Reserved.

#include "UR_HealthBase.h"
#include "UR_Character.h"
#include "UR_HealthComponent.h"

AUR_HealthBase::AUR_HealthBase()
{
	HealAmount = 25;
	bSuperHeal = false;
}

bool AUR_HealthBase::AllowPickupBy_Implementation(class AActor* Other)
{
	if (Super::AllowPickupBy_Implementation(Other))
	{
		// Can always pick up super-heal items
		if (bSuperHeal)
			return true;

		// Can only pick ups health packs if we are below 100
		AUR_Character* Char = Cast<AUR_Character>(Other);
		return (Char && Char->HealthComponent && Char->HealthComponent->Health < Char->HealthComponent->HealthMax);
	}
	return false;
}

void AUR_HealthBase::GiveTo_Implementation(class AActor* Other)
{
	AUR_Character* Char = Cast<AUR_Character>(Other);
	if (Char && Char->HealthComponent)
		Char->HealthComponent->HealBy(HealAmount, bSuperHeal);

	Super::GiveTo_Implementation(Other);
}

FText AUR_HealthBase::GetItemName_Implementation()
{
	return FText::FromString(FString::Printf(TEXT("%i Health"), HealAmount));
}
