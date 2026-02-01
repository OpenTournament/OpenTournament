// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "Misc/EnumRange.h"

#include "UR_PerformanceStatTypes.generated.h"

//////////////////////////////////////////////////////////////////////

// Way to display the stat
UENUM(BlueprintType)
enum class EGameStatDisplayMode : uint8
{
    // Don't show this stat
    Hidden,

    // Show this stat in text form
    TextOnly,

    // Show this stat in graph form
    GraphOnly,

    // Show this stat as both text and graph
    TextAndGraph
};

//////////////////////////////////////////////////////////////////////

// Different kinds of stats that can be displayed on-screen
UENUM(BlueprintType)
enum class EGameDisplayablePerformanceStat : uint8
{
    // stat fps (in Hz)
    ClientFPS,

    // server tick rate (in Hz)
    ServerFPS,

    // idle time spent waiting for vsync or frame rate limit (in seconds)
    IdleTime,

    // Stat unit overall (in seconds)
    FrameTime,

    // Stat unit (game thread, in seconds)
    FrameTime_GameThread,

    // Stat unit (render thread, in seconds)
    FrameTime_RenderThread,

    // Stat unit (RHI thread, in seconds)
    FrameTime_RHIThread,

    // Stat unit (inferred GPU time, in seconds)
    FrameTime_GPU,

    // Network ping (in ms)
    Ping,

    // The incoming packet loss percentage (%)
    PacketLoss_Incoming,

    // The outgoing packet loss percentage (%)
    PacketLoss_Outgoing,

    // The number of packets received in the last second
    PacketRate_Incoming,

    // The number of packets sent in the past second
    PacketRate_Outgoing,

    // The avg. size (in bytes) of packets received
    PacketSize_Incoming,

    // The avg. size (in bytes) of packets sent
    PacketSize_Outgoing,

    // The total latency in MS of the game
    Latency_Total,

    // Game simulation start to driver submission end
    Latency_Game,

    // OS render queue start to GPU render end
    Latency_Render,

    // New stats should go above here
    Count UMETA(Hidden)
};

ENUM_RANGE_BY_COUNT(EGameDisplayablePerformanceStat, EGameDisplayablePerformanceStat::Count);

//////////////////////////////////////////////////////////////////////
