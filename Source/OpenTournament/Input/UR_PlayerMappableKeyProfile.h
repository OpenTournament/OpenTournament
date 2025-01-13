// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UserSettings/EnhancedInputUserSettings.h"

#include "UR_PlayerMappableKeyProfile.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class OPENTOURNAMENT_API UUR_PlayerMappableKeyProfile : public UEnhancedPlayerMappableKeyProfile
{
	GENERATED_BODY()

protected:

	//~ Begin UEnhancedPlayerMappableKeyProfile interface
	virtual void EquipProfile() override;
	virtual void UnEquipProfile() override;
	//~ End UEnhancedPlayerMappableKeyProfile interface
};
