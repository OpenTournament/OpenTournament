// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Components/ControllerComponent.h"

#include "UR_CharacterPartTypes.h"

#include "UR_ControllerComponent_CharacterParts.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class APawn;
class UUR_PawnComponent_CharacterParts;
class UObject;
struct FFrame;

/////////////////////////////////////////////////////////////////////////////////////////////////

enum class ECharacterPartSource : uint8
{
    Natural,

    NaturalSuppressedViaCheat,

    AppliedViaDeveloperSettingsCheat,

    AppliedViaCheatManager
};

/////////////////////////////////////////////////////////////////////////////////////////////////

// A character part requested on a controller component
USTRUCT()
struct FUR_ControllerCharacterPartEntry
{
    GENERATED_BODY()

    FUR_ControllerCharacterPartEntry()
    {}

public:
    // The character part being represented
    UPROPERTY(EditAnywhere, meta=(ShowOnlyInnerProperties))
    FUR_CharacterPart Part;

    // The handle if already applied to a pawn
    FUR_CharacterPartHandle Handle;

    // The source of this part
    ECharacterPartSource Source = ECharacterPartSource::Natural;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

// A component that configure what cosmetic actors to spawn for the owning controller when it possesses a pawn
UCLASS(meta = (BlueprintSpawnableComponent))
class UUR_ControllerComponent_CharacterParts : public UControllerComponent
{
    GENERATED_BODY()

public:
    UUR_ControllerComponent_CharacterParts(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    //~UActorComponent interface
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    //~End of UActorComponent interface

    // Adds a character part to the actor that owns this customization component, should be called on the authority only
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Cosmetics)
    void AddCharacterPart(const FUR_CharacterPart& NewPart);

    // Removes a previously added character part from the actor that owns this customization component, should be called on the authority only
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Cosmetics)
    void RemoveCharacterPart(const FUR_CharacterPart& PartToRemove);

    // Removes all added character parts, should be called on the authority only
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Cosmetics)
    void RemoveAllCharacterParts();

    // Applies relevant developer settings if in PIE
    void ApplyDeveloperSettings();

protected:
    UPROPERTY(EditAnywhere, Category=Cosmetics)
    TArray<FUR_ControllerCharacterPartEntry> CharacterParts;

private:
    UUR_PawnComponent_CharacterParts* GetPawnCustomizer() const;

    UFUNCTION()
    void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

    void AddCharacterPartInternal(const FUR_CharacterPart& NewPart, ECharacterPartSource Source);

    void AddCheatPart(const FUR_CharacterPart& NewPart, bool bSuppressNaturalParts);
    void ClearCheatParts();

    void SetSuppressionOnNaturalParts(bool bSuppressed);

    friend class UUR_CosmeticCheats;
};
