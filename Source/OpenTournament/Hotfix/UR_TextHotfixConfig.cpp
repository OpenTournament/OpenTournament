// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_TextHotfixConfig.h"

#include "Internationalization/PolyglotTextData.h"
#include "Internationalization/TextLocalizationManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_TextHotfixConfig)

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_TextHotfixConfig::UUR_TextHotfixConfig(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_TextHotfixConfig::ApplyTextReplacements() const
{
    FTextLocalizationManager::Get().RegisterPolyglotTextData(TextReplacements);
}

void UUR_TextHotfixConfig::PostInitProperties()
{
    Super::PostInitProperties();
    ApplyTextReplacements();
}

void UUR_TextHotfixConfig::PostReloadConfig(FProperty* PropertyThatWasLoaded)
{
    Super::PostReloadConfig(PropertyThatWasLoaded);
    ApplyTextReplacements();
}

#if WITH_EDITOR
void UUR_TextHotfixConfig::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    ApplyTextReplacements();
}
#endif
