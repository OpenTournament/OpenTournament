// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UserSettings/EnhancedInputUserSettings.h"
#include "PlayerMappableKeySettings.h"

#include "UR_InputUserSettings.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Custom settings class for any input related settings for the OT game.
 * This will be serialized out at the same time as the OT Shared Settings and is
 * compatible with cloud saves through by calling the "Serialize" function.
 */
UCLASS()
class OPENTOURNAMENT_API UUR_InputUserSettings : public UEnhancedInputUserSettings
{
	GENERATED_BODY()
public:
	//~ Begin UEnhancedInputUserSettings interface
	virtual void ApplySettings() override;
	//~ End UEnhancedInputUserSettings interface

	// Add any additional Input Settings here!
	// Some ideas could be:
	// - "toggle vs. hold" to trigger in game actions
	// - aim sensitivity should go here
	// - etc

	// Make sure to mark your properties with the "SaveGame" metadata to have them serialize when saved
	//UPROPERTY(SaveGame, BlueprintReadWrite, Category="Enhanced Input|User Settings")
	// bool bSomeExampleProperty;
};

/**
 * Player Mappable Key settings are settings that are accessible per-action key mapping.
 * This is where you could place additional metadata that may be used by your settings UI,
 * input triggers, or other places where you want to know about a key setting.
 */
UCLASS()
class OPENTOURNAMENT_API UUR_PlayerMappableKeySettings : public UPlayerMappableKeySettings
{
	GENERATED_BODY()

public:

	/** Returns the tooltip that should be displayed on the settings screen for this key */
	const FText& GetTooltipText() const;

protected:
	/** The tooltip that should be associated with this action when displayed on the settings screen */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta=(AllowPrivateAccess=true))
	FText Tooltip = FText::GetEmpty();
};
