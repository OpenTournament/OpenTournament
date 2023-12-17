// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_GameData.h"
#include "UR_AssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_GameData)

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_GameData::UUR_GameData()
{
}

const UUR_GameData& UUR_GameData::UUR_GameData::Get()
{
	return UUR_AssetManager::Get().GetGameData();
}
