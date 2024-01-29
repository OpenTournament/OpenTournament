#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Tickable.h"

class FBUIValidatorModule : public IModuleInterface
{
public:
	// Begin IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual bool SupportsDynamicReloading() override { return true; }
	// End IModuleInterface

protected:
	bool HandleSettingsSaved();

	void OnPostEngineInit();
	void OnObjectReimported( UFactory* ImportFactory, UObject* InObject );
};
