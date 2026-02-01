// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_DamageLogDebuggerComponent.h"

#include "NativeGameplayTags.h"
#include "Engine/World.h"

#include "UR_LogChannels.h"
#include "Messages/GameVerbMessage.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_DamageLogDebuggerComponent)

/////////////////////////////////////////////////////////////////////////////////////////////////

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Game_Damage_Message);

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_DamageLogDebuggerComponent::UUR_DamageLogDebuggerComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.SetTickFunctionEnable(true);
}

void UUR_DamageLogDebuggerComponent::BeginPlay()
{
    Super::BeginPlay();

    UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
    ListenerHandle = MessageSubsystem.RegisterListener(TAG_Game_Damage_Message, this, &ThisClass::OnDamageMessage);
}

void UUR_DamageLogDebuggerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
    MessageSubsystem.UnregisterListener(ListenerHandle);

    Super::EndPlay(EndPlayReason);
}

void UUR_DamageLogDebuggerComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    const double TimeSinceDamage = GetWorld()->GetTimeSeconds() - LastDamageEntryTime;
    if ((TimeSinceDamage >= SecondsBetweenDamageBeforeLogging) && (DamageLog.Num() > 0))
    {
        TArray<FFrameDamageEntry> Entries;
        DamageLog.GenerateValueArray(/*out*/ Entries);
        DamageLog.Reset();

        Entries.Sort
        ([](const FFrameDamageEntry& A, const FFrameDamageEntry& B)
        {
            return A.TimeOfFirstHit < B.TimeOfFirstHit;
        });

        double TotalDamage = 0.0;
        int32 NumImpacts = 0;
        int32 NumFrames = Entries.Num();
        double MinInterval = TNumericLimits<double>::Max();
        double MaxInterval = -TNumericLimits<double>::Max();
        double TotalInterval = 0.0;

        for (int32 i = 0; i < Entries.Num(); ++i)
        {
            FFrameDamageEntry& EntryA = Entries[i];
            NumImpacts += EntryA.NumImpacts;
            TotalDamage += EntryA.SumDamage;

            if (i + 1 < Entries.Num())
            {
                FFrameDamageEntry& EntryB = Entries[i + 1];

                const double TimeGap = EntryB.TimeOfFirstHit - EntryA.TimeOfFirstHit;
                MinInterval = FMath::Min(MinInterval, TimeGap);
                MaxInterval = FMath::Max(MaxInterval, TimeGap);
                TotalInterval += TimeGap;
            }
        }

        UE_LOG
        (
            LogGame,
            Warning,
            TEXT("%d impacts in %d distinct frames over %.2f seconds did %.2f damage"),
            NumImpacts,
            NumFrames,
            TotalInterval,
            TotalDamage
        );
        if (TotalInterval > 0.0)
        {
            UE_LOG
            (
                LogGame,
                Warning,
                TEXT("Interval ranged from %.1f ms to %.1f ms (avg %.1f ms)"),
                MinInterval * 1000.0,
                MaxInterval * 1000.0,
                (MaxInterval + MinInterval) / 2.0 * 1000.0
            );
            UE_LOG(LogGame, Warning, TEXT("DPS %.2f"), TotalDamage / TotalInterval);
        }
        UE_LOG(LogGame, Warning, TEXT("\n"));
    }
}

void UUR_DamageLogDebuggerComponent::OnDamageMessage(FGameplayTag Channel, const FGameVerbMessage& Payload)
{
    if (Payload.Target == GetOwner())
    {
        FFrameDamageEntry& LogEntry = DamageLog.FindOrAdd(GFrameCounter);

        if (LogEntry.TimeOfFirstHit == 0.0)
        {
            LogEntry.TimeOfFirstHit = GetWorld()->GetTimeSeconds();
            LastDamageEntryTime = LogEntry.TimeOfFirstHit;
        }
        LogEntry.NumImpacts++;
        LogEntry.SumDamage += -Payload.Magnitude;
    }
}
