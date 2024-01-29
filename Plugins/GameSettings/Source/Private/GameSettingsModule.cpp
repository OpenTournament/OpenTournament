// Copyright Epic Games, Inc. All Rights Reserved.

#include "Modules/ModuleManager.h"

/**
 * Implements the FGameSettingsModule module.
 */
class FGameSettingsModule : public IModuleInterface
{
public:
	FGameSettingsModule();
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

};


FGameSettingsModule::FGameSettingsModule()
{
}

void FGameSettingsModule::StartupModule()
{
}

void FGameSettingsModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FGameSettingsModule, GameSettings);
