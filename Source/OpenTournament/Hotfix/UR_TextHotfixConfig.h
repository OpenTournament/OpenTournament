// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Engine/DeveloperSettings.h"

#include "UR_TextHotfixConfig.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

struct FPolyglotTextData;
struct FPropertyChangedEvent;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * This class allows hotfixing individual FText values anywhere
 */
UCLASS(config=Game, defaultconfig)
class UUR_TextHotfixConfig : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UUR_TextHotfixConfig(const FObjectInitializer& ObjectInitializer);

    // UObject interface
    virtual void PostInitProperties() override;
    virtual void PostReloadConfig(FProperty* PropertyThatWasLoaded) override;
#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
    // End of UObject interface

private:
    void ApplyTextReplacements() const;

private:
    // The list of FText values to hotfix
    UPROPERTY(Config, EditAnywhere)
    TArray<FPolyglotTextData> TextReplacements;
};
