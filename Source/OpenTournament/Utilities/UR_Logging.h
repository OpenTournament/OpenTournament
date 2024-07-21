// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"

#include "OpenTournament.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations

/////////////////////////////////////////////////////////////////////////////////////////////////

#define DEBUG_LOG(text, ...)                                    DEBUG_LOG_INTERNAL(-1,  FColor::White, 5.0f, text, __VA_ARGS__);
#define DEBUG_LOG_TIME(time, text, ...)                         DEBUG_LOG_INTERNAL(-1,  FColor::White, time, text, __VA_ARGS__);
#define DEBUG_LOG_COLOR(color, text, ...)                       DEBUG_LOG_INTERNAL(-1,  color,         5.0f, text, __VA_ARGS__);
#define DEBUG_LOG_COLOR_TIME(color, time, text, ...)            DEBUG_LOG_INTERNAL(-1,  color,         time, text, __VA_ARGS__);
#define DEBUG_LOG_KEY(key, text, ...)                           DEBUG_LOG_INTERNAL(key, FColor::White, 5.0f, text, __VA_ARGS__);
#define DEBUG_LOG_KEY_TIME(key, time, text, ...)                DEBUG_LOG_INTERNAL(key, FColor::White, time, text, __VA_ARGS__);
#define DEBUG_LOG_KEY_COLOR(key, color, text, ...)              DEBUG_LOG_INTERNAL(key, color,         5.0f, text, __VA_ARGS__);
#define DEBUG_LOG_KEY_COLOR_TIME(key, color, time, text, ...)   DEBUG_LOG_INTERNAL(key, color,         time, text, __VA_ARGS__);

#if !UE_BUILD_SHIPPING
#define DEBUG_LOG_INTERNAL(key, color, time, text, ...) URLogging::LogFormat(TEXT(text), true, true, color, time, key, __VA_ARGS__);
#else
    #define DEBUG_LOG_INTERNAL(color, text, ...)
#endif

namespace URLogging
{
    void Log(FString inText, bool inLogToConsole = true, bool inLogToScreen = true, FColor inColor = FColor::White, float inTimeToDisplay = 5.0f, int32 inKey = -1);

    template <typename FmtType, typename... Types>
    void LogFormat(const FmtType& inFormat, bool inLogToConsole, bool inLogToScreen, FColor inColor, float inTimeToDisplay, int32 inKey, Types... inArgs)
    {
        LogInternal(inFormat, inLogToConsole, inLogToScreen, inColor, inTimeToDisplay, inKey, inArgs...);
    }

    void LogToConsoleInternal(FString inText);

    void LogToScreenInternal(FString inText, FColor inColor = FColor::White, float inTimeToDisplay = 5.0f, int32 inKey = -1);

    template <typename FmtType, typename... Types>
    static void LogInternal(const FmtType& inFormat, bool inLogToConsole, bool inLogToScreen, FColor inColor, float inTimeToDisplay, int32 inKey, Types... inArgs)
    {
        auto text = FString::Printf(inFormat, inArgs...);

        if (inLogToConsole)
        {
            LogToConsoleInternal(text);
        }

        if (inLogToScreen)
        {
            LogToScreenInternal(text, inColor, inTimeToDisplay, inKey);
        }
    }
}
