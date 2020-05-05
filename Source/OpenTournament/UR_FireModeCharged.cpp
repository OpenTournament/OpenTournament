// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

#include "UR_FireModeCharged.h"

#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UUR_FireModeCharged::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(UUR_FireModeCharged, ChargeLevel, COND_InitialOnly);
    DOREPLIFETIME_CONDITION(UUR_FireModeCharged, ChargePausedAt, COND_None);
}

void UUR_FireModeCharged::StartFire_Implementation()
{
}

void UUR_FireModeCharged::StopFire_Implementation()
{
}

float UUR_FireModeCharged::GetTimeUntilIdle_Implementation()
{
    if (bIsBusy)
    {
        if (GetWorld()->GetTimerManager().TimerExists(CooldownTimerHandle))
        {
            return GetWorld()->GetTimerManager().GetTimerRemaining(CooldownTimerHandle);
        }
        else
        {
            return FireInterval;
        }
    }
    return 0.f;
}

float UUR_FireModeCharged::GetCooldownStartTime_Implementation()
{
    if (bIsBusy)
    {
        if (GetWorld()->GetTimerManager().TimerExists(CooldownTimerHandle))
        {
            return GetWorld()->GetTimeSeconds() - GetWorld()->GetTimerManager().GetTimerElapsed(CooldownTimerHandle);
        }
        else
        {
            // future cooldown start time
            return GetWorld()->GetTimeSeconds() + 0.1f;
        }
    }
    return 0.f;
}


float UUR_FireModeCharged::GetContinuousCharge()
{
    return 0.f;
}

void UUR_FireModeCharged::BlockNextCharge(float MaxHoldTime)
{

}

void UUR_FireModeCharged::OnRep_InitialChargeLevel()
{

}
