// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Engine/DeveloperSettingsBackedByCVars.h"

#include "UR_CosmeticDeveloperSettings.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

#define UE_API OPENTOURNAMENT_API

/////////////////////////////////////////////////////////////////////////////////////////////////

class UUR_ExperienceDefinition;

struct FPropertyChangedEvent;
struct FUR_CharacterPart;

/////////////////////////////////////////////////////////////////////////////////////////////////

UENUM()
enum class ECosmeticCheatMode
{
    ReplaceParts,
    AddParts
};

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Cosmetic developer settings / editor cheats
 */
UCLASS(MinimalAPI, Config=EditorPerProjectUserSettings, DisplayName = "Cosmetic Developer Settings")
class UUR_CosmeticDeveloperSettings : public UDeveloperSettingsBackedByCVars
{
    GENERATED_BODY()

public:
    UUR_CosmeticDeveloperSettings();

    //~UDeveloperSettings interface
    virtual FName GetCategoryName() const override;
    //~End of UDeveloperSettings interface

public:
    UPROPERTY(Transient, EditAnywhere)
    TArray<FUR_CharacterPart> CheatCosmeticCharacterParts;

    UPROPERTY(Transient, EditAnywhere)
    ECosmeticCheatMode CheatMode;

#if WITH_EDITOR

public:
    // Called by the editor engine to let us pop reminder notifications when cheats are active
    UE_API void OnPlayInEditorStarted() const;

private:
    void ApplySettings();
    void ReapplyLoadoutIfInPIE();
#endif

public:
    //~UObject interface
#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    virtual void PostReloadConfig(FProperty* PropertyThatWasLoaded) override;
    virtual void PostInitProperties() override;
#endif
    //~End of UObject interface

private:
};

#undef UE_API
