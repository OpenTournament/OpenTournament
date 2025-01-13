// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_AmmoPickup.h"

#include "OpenTournament.h"
#include "UR_Ammo.h"
#include "UR_Character.h"
#include "UR_InventoryComponent.h"

#if WITH_EDITOR
#include <Logging/MessageLog.h>
#include <Misc/UObjectToken.h>
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_AmmoPickup)

/////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "UR_AmmoPickup"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_AmmoPickup::AUR_AmmoPickup(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , AmmoClass_Internal(AUR_Ammo::StaticClass())
    , AmmoAmount(10)
{
    DisplayName = FText::FromString(TEXT("Ammo"));
}

#if WITH_EDITOR
void AUR_AmmoPickup::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (PropertyChangedEvent.GetPropertyName().IsEqual(FName(TEXT("AmmoClass"))))
    {
        DisplayName = AmmoClass_Internal ? AmmoClass_Internal->GetDefaultObject<AUR_Ammo>()->AmmoName : GetDefault<AUR_AmmoPickup>()->DisplayName;
    }
}

void AUR_AmmoPickup::CheckForErrors()
{
    Super::CheckForErrors();

    if (!HasAnyFlags(RF_ClassDefaultObject) && !AmmoClass_Soft)
    {
        const auto ErrorText = FText::Format(LOCTEXT("Missing AmmoClass_Soft Reference", "Null entry at for AmmoClass_Soft in UR_AmmoPickup ({Name})"), FText::FromString(GetNameSafe(this)));

        FMessageLog("MapCheck").Warning()
                               ->AddToken(FUObjectToken::Create(this))
                               ->AddToken(FTextToken::Create(ErrorText));
    }
}
#endif

bool AUR_AmmoPickup::OnPickup_Implementation(AUR_Character* PickupCharacter)
{
    if (AmmoClass_Internal && PickupCharacter && PickupCharacter->InventoryComponent)
    {
        if (HasAuthority())
        {
            PickupCharacter->InventoryComponent->AddAmmo(AmmoClass_Internal, AmmoAmount);
        }
    }

    return Super::OnPickup_Implementation(PickupCharacter);
}

#undef LOCTEXT_NAMESPACE

/////////////////////////////////////////////////////////////////////////////////////////////////
