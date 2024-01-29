// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Subsystems/EngineSubsystem.h"
#include "UR_ExperienceManager.generated.h"

/**
 * Manager for experiences - primarily for arbitration between multiple PIE sessions
 */
UCLASS(MinimalAPI)
class UUR_ExperienceManager : public UEngineSubsystem
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	OPENTOURNAMENT_API void OnPlayInEditorBegun();

	static void NotifyOfPluginActivation(const FString PluginURL);
	static bool RequestToDeactivatePlugin(const FString PluginURL);
#else
	static void NotifyOfPluginActivation(const FString PluginURL) {}
	static bool RequestToDeactivatePlugin(const FString PluginURL) { return true; }
#endif

private:
	// The map of requests to active count for a given game feature plugin
	// (to allow first in, last out activation management during PIE)
	TMap<FString, int32> GameFeaturePluginRequestCountMap;
};
