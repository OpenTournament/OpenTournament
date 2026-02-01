// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_PerformanceStatSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/NetConnection.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "ProfilingDebugging/CsvProfiler.h"

#include "GameModes/UR_GameState.h"
#include "Performance/LatencyMarkerModule.h"
#include "Performance/UR_PerformanceStatTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_PerformanceStatSubsystem)

/////////////////////////////////////////////////////////////////////////////////////////////////

CSV_DEFINE_CATEGORY(GamePerformance, /*bIsEnabledByDefault=*/false);

/////////////////////////////////////////////////////////////////////////////////////////////////

class FSubsystemCollectionBase;

//////////////////////////////////////////////////////////////////////
// FGamePerformanceStatCache

void FGamePerformanceStatCache::StartCharting()
{}

void FGamePerformanceStatCache::ProcessFrame(const FFrameData& FrameData)
{
    // Record stats about the frame data
    {
        RecordStat
        (
            EGameDisplayablePerformanceStat::ClientFPS,
            (FrameData.TrueDeltaSeconds != 0.0) ? 1.0 / FrameData.TrueDeltaSeconds : 0.0);

        RecordStat(EGameDisplayablePerformanceStat::IdleTime, FrameData.IdleSeconds);
        RecordStat(EGameDisplayablePerformanceStat::FrameTime, FrameData.TrueDeltaSeconds);
        RecordStat(EGameDisplayablePerformanceStat::FrameTime_GameThread, FrameData.GameThreadTimeSeconds);
        RecordStat(EGameDisplayablePerformanceStat::FrameTime_RenderThread, FrameData.RenderThreadTimeSeconds);
        RecordStat(EGameDisplayablePerformanceStat::FrameTime_RHIThread, FrameData.RHIThreadTimeSeconds);
        RecordStat(EGameDisplayablePerformanceStat::FrameTime_GPU, FrameData.GPUTimeSeconds);
    }

    if (UWorld* World = MySubsystem->GetGameInstance()->GetWorld())
    {
        // Record some networking related stats
        if (const AUR_GameState* GameState = World->GetGameState<AUR_GameState>())
        {
            RecordStat(EGameDisplayablePerformanceStat::ServerFPS, GameState->GetServerFPS());
        }

        if (APlayerController* LocalPC = GEngine->GetFirstLocalPlayerController(World))
        {
            if (APlayerState* PS = LocalPC->GetPlayerState<APlayerState>())
            {
                RecordStat(EGameDisplayablePerformanceStat::Ping, PS->GetPingInMilliseconds());
            }

            if (UNetConnection* NetConnection = LocalPC->GetNetConnection())
            {
                const UNetConnection::FNetConnectionPacketLoss& InLoss = NetConnection->GetInLossPercentage();
                RecordStat(EGameDisplayablePerformanceStat::PacketLoss_Incoming, InLoss.GetAvgLossPercentage());

                const UNetConnection::FNetConnectionPacketLoss& OutLoss = NetConnection->GetOutLossPercentage();
                RecordStat(EGameDisplayablePerformanceStat::PacketLoss_Outgoing, OutLoss.GetAvgLossPercentage());

                RecordStat(EGameDisplayablePerformanceStat::PacketRate_Incoming, NetConnection->InPacketsPerSecond);
                RecordStat(EGameDisplayablePerformanceStat::PacketRate_Outgoing, NetConnection->OutPacketsPerSecond);

                RecordStat(EGameDisplayablePerformanceStat::PacketSize_Incoming, (NetConnection->InPacketsPerSecond != 0) ? (NetConnection->InBytesPerSecond / (float)NetConnection->InPacketsPerSecond) : 0.0f);
                RecordStat(EGameDisplayablePerformanceStat::PacketSize_Outgoing, (NetConnection->OutPacketsPerSecond != 0) ? (NetConnection->OutBytesPerSecond / (float)NetConnection->OutPacketsPerSecond) : 0.0f);
            }

            // Finally, record some input latency related stats if they are enabled
            TArray<ILatencyMarkerModule*> LatencyMarkerModules = IModularFeatures::Get().GetModularFeatureImplementations<ILatencyMarkerModule>(ILatencyMarkerModule::GetModularFeatureName());
            for (ILatencyMarkerModule* LatencyMarkerModule : LatencyMarkerModules)
            {
                if (LatencyMarkerModule->GetEnabled())
                {
                    const float TotalLatencyMs = LatencyMarkerModule->GetTotalLatencyInMs();
                    if (TotalLatencyMs > 0.0f)
                    {
                        // Record some stats about the latency of the game
                        RecordStat(EGameDisplayablePerformanceStat::Latency_Total, TotalLatencyMs);
                        RecordStat(EGameDisplayablePerformanceStat::Latency_Game, LatencyMarkerModule->GetGameLatencyInMs());
                        RecordStat(EGameDisplayablePerformanceStat::Latency_Render, LatencyMarkerModule->GetRenderLatencyInMs());

                        // Record some CSV profile stats.
                        // You can see these by using the following commands
                        // Start and stop the profile:
                        //	CsvProfile Start
                        //	CsvProfile Stop
                        //
                        // Or, you can profile for a certain number of frames:
                        // CsvProfile Frames=10
                        //
                        // And this will output a .csv file to the Saved\Profiling\CSV folder
#if CSV_PROFILER
                        if (FCsvProfiler* Profiler = FCsvProfiler::Get())
                        {
                            static const FName TotalLatencyStatName = TEXT("Game_Latency_Total");
                            Profiler->RecordCustomStat(TotalLatencyStatName, CSV_CATEGORY_INDEX(GamePerformance), TotalLatencyMs, ECsvCustomStatOp::Set);

                            static const FName GameLatencyStatName = TEXT("Game_Latency_Game");
                            Profiler->RecordCustomStat(GameLatencyStatName, CSV_CATEGORY_INDEX(GamePerformance), LatencyMarkerModule->GetGameLatencyInMs(), ECsvCustomStatOp::Set);

                            static const FName RenderLatencyStatName = TEXT("Game_Latency_Render");
                            Profiler->RecordCustomStat(RenderLatencyStatName, CSV_CATEGORY_INDEX(GamePerformance), LatencyMarkerModule->GetRenderLatencyInMs(), ECsvCustomStatOp::Set);
                        }
#endif

                        // Some more fine grain latency numbers can be found on the marker module if desired
                        //LatencyMarkerModule->GetRenderLatencyInMs()));
                        //LatencyMarkerModule->GetDriverLatencyInMs()));
                        //LatencyMarkerModule->GetOSRenderQueueLatencyInMs()));
                        //LatencyMarkerModule->GetGPURenderLatencyInMs()));
                        break;
                    }
                }
            }
        }
    }
}

void FGamePerformanceStatCache::StopCharting()
{}

void FGamePerformanceStatCache::RecordStat(const EGameDisplayablePerformanceStat Stat, const double Value)
{
    PerfStateCache.FindOrAdd(Stat).RecordSample(Value);
}

double FGamePerformanceStatCache::GetCachedStat(EGameDisplayablePerformanceStat Stat) const
{
    static_assert((int32)EGameDisplayablePerformanceStat::Count == 18, "Need to update this function to deal with new performance stats");

    if (const FSampledStatCache* Cache = GetCachedStatData(Stat))
    {
        return Cache->GetLastCachedStat();
    }

    return 0.0;
}

const FSampledStatCache* FGamePerformanceStatCache::GetCachedStatData(const EGameDisplayablePerformanceStat Stat) const
{
    static_assert((int32)EGameDisplayablePerformanceStat::Count == 18, "Need to update this function to deal with new performance stats");

    return PerfStateCache.Find(Stat);
}

//////////////////////////////////////////////////////////////////////
// UUR_PerformanceStatSubsystem

void UUR_PerformanceStatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Tracker = MakeShared<FGamePerformanceStatCache>(this);
    GEngine->AddPerformanceDataConsumer(Tracker);
}

void UUR_PerformanceStatSubsystem::Deinitialize()
{
    GEngine->RemovePerformanceDataConsumer(Tracker);
    Tracker.Reset();
}

double UUR_PerformanceStatSubsystem::GetCachedStat(EGameDisplayablePerformanceStat Stat) const
{
    return Tracker->GetCachedStat(Stat);
}

const FSampledStatCache* UUR_PerformanceStatSubsystem::GetCachedStatData(const EGameDisplayablePerformanceStat Stat) const
{
    return Tracker->GetCachedStatData(Stat);
}
