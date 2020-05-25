// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

#include "UR_FireModeBasic.h"

#include "Engine/World.h"
#include "TimerManager.h"

void UUR_FireModeBasic::StartFire_Implementation()
{
    if (!bRequestedFire)
    {
        // We don't want server to auto-fire after spinup routine.
        // FireModeBasic shots are all individually initiated by client (or the fire requester).
        return;
    }
    if (GetWorld()->GetTimerManager().GetTimerRemaining(CooldownTimerHandle) > 0.f)
    {
        return; // fire loop is already active
    }

    SetBusy(true);

    FSimulatedShotInfo SimulatedInfo;

    if (BasicInterface)
    {
        IUR_FireModeBasicInterface::Execute_PlayFireEffects(BasicInterface.GetObject(), this);

        if (bIsHitscan)
        {
            FHitscanVisualInfo HitscanInfo;
            IUR_FireModeBasicInterface::Execute_SimulateHitscanShot(BasicInterface.GetObject(), this, SimulatedInfo, HitscanInfo);
            IUR_FireModeBasicInterface::Execute_PlayHitscanEffects(BasicInterface.GetObject(), this, HitscanInfo);
        }
        else
        {
            IUR_FireModeBasicInterface::Execute_SimulateShot(BasicInterface.GetObject(), this, SimulatedInfo);
        }
    }

    if (GetNetMode() == NM_Client)
    {
        LocalFireTime = GetWorld()->GetTimeSeconds();
        GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, this, &UUR_FireModeBasic::CooldownTimer, FMath::Max(FireInterval, 0.001f), false);
    }

    ServerFire(SimulatedInfo);
}

void UUR_FireModeBasic::CooldownTimer()
{
    if (bRequestedIdle || !bRequestedFire)
    {
        if (GetTimeUntilIdle() <= 0.f)  //Spindown support
        {
            SetBusy(false);
        }
    }
    else if (SpinUpTime <= 0.f || bFullySpinnedUp)
    {
        StartFire();
    }
}

float UUR_FireModeBasic::GetTimeUntilIdle_Implementation()
{
    if (bIsBusy)
    {
        return FMath::Max(
            Super::GetTimeUntilIdle_Implementation(),
            GetWorld()->GetTimerManager().GetTimerRemaining(CooldownTimerHandle)
        );
    }
    return 0.f;
}

float UUR_FireModeBasic::GetCooldownStartTime_Implementation()
{
    // Use either CooldownTimerHandle or SpinDownIdleTimerHandle depending on which will trigger LAST.
    if (GetWorld()->GetTimerManager().GetTimerRemaining(SpinDownIdleTimerHandle) > GetWorld()->GetTimerManager().GetTimerRemaining(CooldownTimerHandle))
    {
        return Super::GetCooldownStartTime_Implementation();
    }
    else if ( GetWorld()->GetTimerManager().TimerExists(CooldownTimerHandle))
    {
        return GetWorld()->GetTimeSeconds() - GetWorld()->GetTimerManager().GetTimerElapsed(CooldownTimerHandle);
    }
    return 0.f;
}

void UUR_FireModeBasic::ServerFire_Implementation(const FSimulatedShotInfo& SimulatedInfo)
{
    float Delay;

    if (SpinUpTime > 0.f)
    {
        //NOTE: If we have spinup, the weapon timings validation is already done on ServerSpinUp.
        // We only need to check internal spinup and cooldown timers.
        if (!bFullySpinnedUp)
        {
            if (GetWorld()->GetTimerManager().IsTimerActive(SpinUpTimerHandle))
            {
                Delay = GetWorld()->GetTimerManager().GetTimerRemaining(SpinUpTimerHandle);
            }
            else
            {
                Delay = 1.f;
            }
        }
        else
        {
            Delay = GetWorld()->GetTimerManager().GetTimerRemaining(CooldownTimerHandle);
        }
    }
    else if (BaseInterface)
    {
        Delay = IUR_FireModeBaseInterface::Execute_TimeUntilReadyToFire(BaseInterface.GetObject(), this);
    }
    else
    {
        Delay = GetTimeUntilIdle();
    }

    if (Delay > 0.f)
    {
        UE_LOG(LogWeapon, Log, TEXT("ServerFire Delay = %f"), Delay);

        if (Delay > FMath::Min(0.200f, FireInterval / 2.f))
        {
            // Too much delay, discard this shot
            return;
        }

        // Delay a bit and fire
        FTimerDelegate Callback;
        Callback.BindLambda([this, SimulatedInfo]
        {
            ServerFire_Implementation(SimulatedInfo);
        });
        GetWorld()->GetTimerManager().SetTimer(DelayedFireTimerHandle, Callback, Delay, false);
        return;
    }

    AuthorityShot(SimulatedInfo);
}

void UUR_FireModeBasic::AuthorityShot(const FSimulatedShotInfo& SimulatedInfo)
{
    SetBusy(true);

    if (bIsHitscan)
    {
        FHitscanVisualInfo HitscanInfo;
        if (BasicInterface)
        {
            IUR_FireModeBasicInterface::Execute_AuthorityHitscanShot(BasicInterface.GetObject(), this, SimulatedInfo, HitscanInfo);
        }
        MulticastFiredHitscan(HitscanInfo);
    }
    else
    {
        if (BasicInterface)
        {
            IUR_FireModeBasicInterface::Execute_AuthorityShot(BasicInterface.GetObject(), this, SimulatedInfo);
        }
        MulticastFired();
    }

    GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, this, &UUR_FireModeBasic::CooldownTimer, FMath::Max(FireInterval, 0.001f), false);
}

void UUR_FireModeBasic::MulticastFired_Implementation()
{
    if (GetNetMode() == NM_Client)
    {
        if (LocalFireTime > 0.f)
        {
            LocalConfirmFired();
        }
        else
        {
            // Set busy+cooldown on remote clients as well so they can track state accurately
            SetBusy(true);
            GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, this, &UUR_FireModeBasic::CooldownTimer, FMath::Max(FireInterval, 0.001f), false);
            // Remote clients visual callback
            if (BasicInterface)
            {
                IUR_FireModeBasicInterface::Execute_PlayFireEffects(BasicInterface.GetObject(), this);
            }
        }
    }
}

void UUR_FireModeBasic::MulticastFiredHitscan_Implementation(const FHitscanVisualInfo& HitscanInfo)
{
    if (GetNetMode() == NM_Client)
    {
        if (LocalFireTime > 0.f)
        {
            LocalConfirmFired();
        }
        else
        {
            // Set busy+cooldown on remote clients as well so they can track state accurately
            SetBusy(true);
            GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, this, &UUR_FireModeBasic::CooldownTimer, FMath::Max(FireInterval, 0.001f), false);
            // Remote clients visual callbacks
            if (BasicInterface)
            {
                IUR_FireModeBasicInterface::Execute_PlayFireEffects(BasicInterface.GetObject(), this);
                IUR_FireModeBasicInterface::Execute_PlayHitscanEffects(BasicInterface.GetObject(), this, HitscanInfo);
            }
        }
    }
}

void UUR_FireModeBasic::LocalConfirmFired()
{
    // Server just fired, adjust our fire loop accordingly
    if (bIsBusy)
    {
        float FirePing = GetWorld()->TimeSince(LocalFireTime);
        float Delay = FireInterval - FirePing / 2.f;
        if (Delay > 0.f)
        {
            GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, this, &UUR_FireModeBasic::CooldownTimer, Delay, false);
        }
    }

    // Don't keep this var around
    LocalFireTime = 0.f;
}
