// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Logging/LogMacros.h"

class UObject;

/////////////////////////////////////////////////////////////////////////////////////////////////

DECLARE_LOG_CATEGORY_EXTERN(LogGame, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogGameExperience, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogGameUI, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogNetOT, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogWeapon, Log, All);

#define GAME_PRINT(Time, Color, Message, ...)(GEngine->AddOnScreenDebugMessage(-1, Time, Color, *FString::Printf(TEXT(Message), ##__VA_ARGS__)))

#define GAME_LOG(Category, Level, Message, ...) UE_LOG(Category, Level, TEXT("[%s](Line: %d): %s"), *FString(__FUNCTION__), __LINE__, *FString::Printf(TEXT(Message), ##__VA_ARGS__))

OPENTOURNAMENT_API FString GetClientServerContextString(UObject* ContextObject = nullptr);
