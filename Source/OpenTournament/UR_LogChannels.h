// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Logging/LogMacros.h"

class UObject;

/////////////////////////////////////////////////////////////////////////////////////////////////

DECLARE_LOG_CATEGORY_EXTERN(LogGame, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogGameExperience, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogGameAbilitySystem, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogGameTeams, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogGameUI, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogNetOT, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogWeapon, Log, All);

#define GAME_PRINT(Time, Color, Message, ...)(GEngine->AddOnScreenDebugMessage(-1, Time, Color, *FString::Printf(TEXT(Message), ##__VA_ARGS__)))

#define GAME_LOG(Category, Level, Message, ...) UE_LOG(Category, Level, TEXT("[%s](Line: %d): %s"), *FString(__FUNCTION__), __LINE__, *FString::Printf(TEXT(Message), ##__VA_ARGS__))

#define GAME_LOG_EX(WorldContextObject, Category, Level, Message, ...) \
{ \
    FString NetModeStr; \
    if (GEngine) \
    { \
        UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject); \
        if (World) \
        { \
            ENetMode NetMode = World->GetNetMode(); \
            switch (NetMode) \
            { \
                case NM_Client: \
                    NetModeStr = TEXT("Client"); \
                    break; \
                case NM_ListenServer: \
                    NetModeStr = TEXT("ListenServer"); \
                    break; \
                case NM_DedicatedServer: \
                    NetModeStr = TEXT("DedicatedServer"); \
                    break; \
                case NM_Standalone: \
                    NetModeStr = TEXT("Standalone"); \
                    break; \
                default: \
                    NetModeStr = TEXT("Unknown"); \
            } \
        } \
    } \
    else \
    { \
        NetModeStr = TEXT("Unknown"); \
    } \
    UE_LOG(Category, Level, TEXT("[%s](Line: %d) [%s]: %s"), *FString(__FUNCTION__), __LINE__, *NetModeStr, *FString::Printf(TEXT(Message), ##__VA_ARGS__)); \
}

OPENTOURNAMENT_API FString GetClientServerContextString(UObject* ContextObject = nullptr);
