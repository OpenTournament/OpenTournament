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


//============================================================
// Core
//============================================================

void UUR_FireModeCharged::StartFire_Implementation()
{
    StartCharge();
}

void UUR_FireModeCharged::StartCharge()
{
    if (bIsBusy)
    {
        return; // already charging, or holding full charge, or in cooldown
    }

    //Replicate
    if (GetNetMode() == NM_Client && UUR_FunctionLibrary::IsComponentLocallyControlled(this))
    {
        LocalStartChargeTime = GetWorld()->GetTimeSeconds();
        ServerStartCharge();
    }
    else if (GetOwnerRole() == ROLE_Authority)
    {
        // Replicate to remotes
        MulticastStartCharge();
    }

    SetBusy(true);

    ChargeLevel = 0;

    // First charge is immediate
    NextChargeLevel();
}

void UUR_FireModeCharged::NextChargeLevel()
{
    bool bWasPaused = (ChargePausedAt > 0);

    if (bWasPaused)
    {
        ChargeLevel = ChargePausedAt;
    }
    else
    {
        ChargeLevel++;
    }

    // In case of infinite charge, block must be called again at every ChargeLevel callback.
    // This only concerns authority. Clients receive ChargePausedAt via replication.
    if (GetOwnerRole() == ROLE_Authority)
    {
        ChargePausedAt = 0;
    }

    // Prep next charge
    if (ChargeLevel < MaxChargeLevel)
    {
        float Delay = FMath::Max(ChargeInterval, 0.001f);
        GetWorld()->GetTimerManager().SetTimer(ChargeTimerHandle, this, &UUR_FireModeCharged::NextChargeLevel, Delay, false);
    }

    // Interface callback, which may call BlockNextCharge(), which may cancel the charge
    if (ChargedInterface)
    {
        IUR_FireModeChargedInterface::Execute_ChargeLevel(ChargedInterface.GetObject(), this, ChargeLevel, bWasPaused);
    }

    // If max charge, and if callback did not set hold timeout, do it
    if (ChargeLevel == MaxChargeLevel && ChargePausedAt == 0)
    {
        SetHoldTimeout(MaxChargeHoldTime);
    }
}

void UUR_FireModeCharged::BlockNextCharge(float MaxHoldTime)
{
    ChargePausedAt = ChargeLevel;
    SetHoldTimeout(MaxHoldTime);
}

void UUR_FireModeCharged::SetHoldTimeout(float MaxHoldTime)
{
    if (MaxHoldTime >= 0.f)
    {
        // Stop charging
        GetWorld()->GetTimerManager().ClearTimer(ChargeTimerHandle);

        if (GetNetMode() == NM_DedicatedServer)
        {
            // On server, allow an extra 200ms to receive the firing packet from client, before enforcing
            MaxHoldTime += 0.200f;
        }
        if (MaxHoldTime > 0.f)
        {
            GetWorld()->GetTimerManager().SetTimer(ChargeTimerHandle, this, &UUR_FireModeCharged::HoldTimeout, MaxHoldTime, false);
        }
        else
        {
            HoldTimeout();
        }
    }
    else
    {
        //infinite hold... what do?
    }
}

void UUR_FireModeCharged::HoldTimeout()
{
    ReleaseCharge(true);
}

void UUR_FireModeCharged::StopFire_Implementation()
{
    ReleaseCharge();

    Super::StopFire_Implementation();
}

void UUR_FireModeCharged::ReleaseCharge(bool bOvercharged)
{
    if (ChargeLevel > 0)
    {
        if (bOvercharged && ChargedInterface)
        {
            IUR_FireModeChargedInterface::Execute_Overcharged(ChargedInterface.GetObject(), this);
        }

        // Fire the charged shot
        bRequestedFire = true;
        Super::StartFire_Implementation();
        bRequestedFire = false;

        GetWorld()->GetTimerManager().ClearTimer(ChargeTimerHandle);
        ChargeLevel = 0;
    }
}

void UUR_FireModeCharged::CooldownTimer()
{
    // FireModeCharged always goes back to Idle between every shot.
    SetBusy(false);
}


//============================================================
// Replication
//============================================================

void UUR_FireModeCharged::ServerStartCharge_Implementation()
{
    // Validation
    if (BaseInterface)
    {
        float Delay = IUR_FireModeBaseInterface::Execute_TimeUntilReadyToFire(BaseInterface.GetObject(), this);
        if (Delay > 0.f)
        {
            UE_LOG(LogWeapon, Log, TEXT("ServerStartCharge Delay = %f"), Delay);
            if (Delay < TIMEUNTILFIRE_NEVER)
            {
                GetWorld()->GetTimerManager().SetTimer(DelayedFireTimerHandle, this, &UUR_FireModeCharged::ServerStartCharge_Implementation, Delay, false);
            }
            return;
        }
    }

    StartCharge();
}

void UUR_FireModeCharged::MulticastStartCharge_Implementation()
{
    if (GetNetMode() == NM_Client)
    {
        if (LocalStartChargeTime > 0.f)
        {
            // this is not really used, just to differentiate the instigator client vs. other remotes.
            LocalStartChargeTime = 0.f;
        }
        else
        {
            StartCharge();
        }
    }
}

void UUR_FireModeCharged::OnRep_InitialChargeLevel()
{
    if (GetNetMode() == NM_Client)
    {
        if (ChargeLevel == 1)
        {
            // Start from beginning
            StartCharge();
        }
        else
        {
            // Start the loop midway
            SetBusy(true);
            ChargeLevel--;
            NextChargeLevel();
        }
    }
}

void UUR_FireModeCharged::ServerFire_Implementation(const FSimulatedShotInfo& SimulatedInfo)
{
    // We don't need all the FireModeBasic validations here, we do validations on ServerStartCharge.
    if (ChargeLevel > 0)
    {
        AuthorityShot(SimulatedInfo);
    }
}

void UUR_FireModeCharged::MulticastFired_Implementation()
{
    Super::MulticastFired_Implementation();

    GetWorld()->GetTimerManager().ClearTimer(ChargeTimerHandle);
    ChargeLevel = 0;
    ChargePausedAt = 0;
}

void UUR_FireModeCharged::MulticastFiredHitscan_Implementation(const FHitscanVisualInfo& HitscanInfo)
{
    Super::MulticastFiredHitscan_Implementation(HitscanInfo);

    GetWorld()->GetTimerManager().ClearTimer(ChargeTimerHandle);
    ChargeLevel = 0;
    ChargePausedAt = 0;
}


//============================================================
// Utilities
//============================================================

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

float UUR_FireModeCharged::GetTotalChargePercent(bool bIncludePartial)
{
    if (ChargeLevel >= MaxChargeLevel)
    {
        return 1.f;
    }
    if (ChargeLevel > 0)
    {
        float OneCharge = 1.f / (float)(MaxChargeLevel - 1);

        float Integral = (float)(ChargeLevel - 1) * OneCharge;

        if (!bIncludePartial || ChargePausedAt > 0)
        {
            return Integral;
        }

        float PartialPct = 1.f - GetWorld()->GetTimerManager().GetTimerRemaining(ChargeTimerHandle) / ChargeInterval;

        return Integral + PartialPct * OneCharge;
    }

    return 0.f;
}
