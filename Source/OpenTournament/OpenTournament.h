// Copyright 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

DECLARE_LOG_CATEGORY_EXTERN(Game, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(GameUI, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(Net, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(Weapon, Log, All);

/////////////////////////////////////////////////////////////////////////////////////////////////

#define GAME_PRINT(Time, Color, Message, ...) (GEngine->AddOnScreenDebugMessage(-1, Time, Color, *FString::Printf(TEXT(Message), ##__VA_ARGS__)))

#define GAME_LOG(Category, Level, Message, ...) UE_LOG(Category, Level, TEXT("[%s](Line: %d): %s"), *FString(__FUNCTION__), __LINE__, *FString::Printf(TEXT(Message), ##__VA_ARGS__))