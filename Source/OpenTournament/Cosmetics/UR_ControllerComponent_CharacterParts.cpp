// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "Cosmetics/UR_ControllerComponent_CharacterParts.h"

#include "GameFramework/CheatManagerDefines.h"
#include "GameFramework/Pawn.h"

#include "UR_CosmeticDeveloperSettings.h"
#include "Cosmetics/UR_CharacterPartTypes.h"
#include "Cosmetics/UR_PawnComponent_CharacterParts.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_ControllerComponent_CharacterParts)

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_ControllerComponent_CharacterParts::UUR_ControllerComponent_CharacterParts(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_ControllerComponent_CharacterParts::BeginPlay()
{
    Super::BeginPlay();

    // Listen for pawn possession changed events
    if (HasAuthority())
    {
        if (AController* OwningController = GetController<AController>())
        {
            OwningController->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnPossessedPawnChanged);

            if (APawn* ControlledPawn = GetPawn<APawn>())
            {
                OnPossessedPawnChanged(nullptr, ControlledPawn);
            }
        }

        ApplyDeveloperSettings();
    }
}

void UUR_ControllerComponent_CharacterParts::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    RemoveAllCharacterParts();
    Super::EndPlay(EndPlayReason);
}

UUR_PawnComponent_CharacterParts* UUR_ControllerComponent_CharacterParts::GetPawnCustomizer() const
{
    if (APawn* ControlledPawn = GetPawn<APawn>())
    {
        return ControlledPawn->FindComponentByClass<UUR_PawnComponent_CharacterParts>();
    }
    return nullptr;
}

void UUR_ControllerComponent_CharacterParts::AddCharacterPart(const FUR_CharacterPart& NewPart)
{
    AddCharacterPartInternal(NewPart, ECharacterPartSource::Natural);
}

void UUR_ControllerComponent_CharacterParts::AddCharacterPartInternal(const FUR_CharacterPart& NewPart, ECharacterPartSource Source)
{
    FUR_ControllerCharacterPartEntry& NewEntry = CharacterParts.AddDefaulted_GetRef();
    NewEntry.Part = NewPart;
    NewEntry.Source = Source;

    if (UUR_PawnComponent_CharacterParts* PawnCustomizer = GetPawnCustomizer())
    {
        if (NewEntry.Source != ECharacterPartSource::NaturalSuppressedViaCheat)
        {
            NewEntry.Handle = PawnCustomizer->AddCharacterPart(NewPart);
        }
    }
}

void UUR_ControllerComponent_CharacterParts::RemoveCharacterPart(const FUR_CharacterPart& PartToRemove)
{
    for (auto EntryIt = CharacterParts.CreateIterator(); EntryIt; ++EntryIt)
    {
        if (FUR_CharacterPart::AreEquivalentParts(EntryIt->Part, PartToRemove))
        {
            if (UUR_PawnComponent_CharacterParts* PawnCustomizer = GetPawnCustomizer())
            {
                PawnCustomizer->RemoveCharacterPart(EntryIt->Handle);
            }

            EntryIt.RemoveCurrent();
            break;
        }
    }
}

void UUR_ControllerComponent_CharacterParts::RemoveAllCharacterParts()
{
    if (UUR_PawnComponent_CharacterParts* PawnCustomizer = GetPawnCustomizer())
    {
        for (FUR_ControllerCharacterPartEntry& Entry : CharacterParts)
        {
            PawnCustomizer->RemoveCharacterPart(Entry.Handle);
        }
    }

    CharacterParts.Reset();
}

void UUR_ControllerComponent_CharacterParts::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
    // Remove from the old pawn
    if (UUR_PawnComponent_CharacterParts* OldCustomizer = OldPawn ? OldPawn->FindComponentByClass<UUR_PawnComponent_CharacterParts>() : nullptr)
    {
        for (FUR_ControllerCharacterPartEntry& Entry : CharacterParts)
        {
            OldCustomizer->RemoveCharacterPart(Entry.Handle);
            Entry.Handle.Reset();
        }
    }

    // Apply to the new pawn
    if (UUR_PawnComponent_CharacterParts* NewCustomizer = NewPawn ? NewPawn->FindComponentByClass<UUR_PawnComponent_CharacterParts>() : nullptr)
    {
        for (FUR_ControllerCharacterPartEntry& Entry : CharacterParts)
        {
            // Don't readd if it's already there, this can get called with a null oldpawn
            if (!Entry.Handle.IsValid() && Entry.Source != ECharacterPartSource::NaturalSuppressedViaCheat)
            {
                Entry.Handle = NewCustomizer->AddCharacterPart(Entry.Part);
            }
        }
    }
}

void UUR_ControllerComponent_CharacterParts::ApplyDeveloperSettings()
{
#if UE_WITH_CHEAT_MANAGER
    const UUR_CosmeticDeveloperSettings* Settings = GetDefault<UUR_CosmeticDeveloperSettings>();

    // Suppress or unsuppress natural parts if needed
    const bool bSuppressNaturalParts = (Settings->CheatMode == ECosmeticCheatMode::ReplaceParts) && (Settings->CheatCosmeticCharacterParts.Num() > 0);
    SetSuppressionOnNaturalParts(bSuppressNaturalParts);

    // Remove anything added by developer settings and re-add it
    UUR_PawnComponent_CharacterParts* PawnCustomizer = GetPawnCustomizer();
    for (auto It = CharacterParts.CreateIterator(); It; ++It)
    {
        if (It->Source == ECharacterPartSource::AppliedViaDeveloperSettingsCheat)
        {
            if (PawnCustomizer != nullptr)
            {
                PawnCustomizer->RemoveCharacterPart(It->Handle);
            }
            It.RemoveCurrent();
        }
    }

    // Add new parts
    for (const FUR_CharacterPart& PartDesc : Settings->CheatCosmeticCharacterParts)
    {
        AddCharacterPartInternal(PartDesc, ECharacterPartSource::AppliedViaDeveloperSettingsCheat);
    }
#endif
}


void UUR_ControllerComponent_CharacterParts::AddCheatPart(const FUR_CharacterPart& NewPart, bool bSuppressNaturalParts)
{
#if UE_WITH_CHEAT_MANAGER
    SetSuppressionOnNaturalParts(bSuppressNaturalParts);
    AddCharacterPartInternal(NewPart, ECharacterPartSource::AppliedViaCheatManager);
#endif
}

void UUR_ControllerComponent_CharacterParts::ClearCheatParts()
{
#if UE_WITH_CHEAT_MANAGER
    UUR_PawnComponent_CharacterParts* PawnCustomizer = GetPawnCustomizer();

    // Remove anything added by cheat manager cheats
    for (auto It = CharacterParts.CreateIterator(); It; ++It)
    {
        if (It->Source == ECharacterPartSource::AppliedViaCheatManager)
        {
            if (PawnCustomizer != nullptr)
            {
                PawnCustomizer->RemoveCharacterPart(It->Handle);
            }
            It.RemoveCurrent();
        }
    }

    ApplyDeveloperSettings();
#endif
}

void UUR_ControllerComponent_CharacterParts::SetSuppressionOnNaturalParts(bool bSuppressed)
{
#if UE_WITH_CHEAT_MANAGER
    UUR_PawnComponent_CharacterParts* PawnCustomizer = GetPawnCustomizer();

    for (FUR_ControllerCharacterPartEntry& Entry : CharacterParts)
    {
        if ((Entry.Source == ECharacterPartSource::Natural) && bSuppressed)
        {
            // Suppress
            if (PawnCustomizer != nullptr)
            {
                PawnCustomizer->RemoveCharacterPart(Entry.Handle);
                Entry.Handle.Reset();
            }
            Entry.Source = ECharacterPartSource::NaturalSuppressedViaCheat;
        }
        else if ((Entry.Source == ECharacterPartSource::NaturalSuppressedViaCheat) && !bSuppressed)
        {
            // Unsuppress
            if (PawnCustomizer != nullptr)
            {
                Entry.Handle = PawnCustomizer->AddCharacterPart(Entry.Part);
            }
            Entry.Source = ECharacterPartSource::Natural;
        }
    }
#endif
}
