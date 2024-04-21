// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_FireModeBase.h"

#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "UR_FunctionLibrary.h"
#include "UR_LogChannels.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_FireModeBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(ThisClass, bIsSpinningUpRep, COND_SkipOwner);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_FireModeBase::SetBusy(bool bNewBusy)
{
    if (bNewBusy != bIsBusy)
    {
        bIsBusy = bNewBusy;

        if (!bIsBusy)
        {
            bRequestedIdle = false;
        }

        if (BaseInterface)
        {
            IUR_FireModeBaseInterface::Execute_FireModeChangedStatus(BaseInterface.GetObject(), this);
        }
    }
}


//============================================================
// StartFire / SpinUp
//============================================================

void UUR_FireModeBase::RequestStartFire_Implementation()
{
    bRequestedFire = true;

    if (SpinUpTime > 0.f)
    {
        SpinUp();
    }
    else
    {
        StartFire();
    }
}

void UUR_FireModeBase::SpinUp()
{
    if (bIsBusy && bRequestedIdle)
    {
        return; //don't spinup again if we requested idle
    }
    if (bFullySpinnedUp || GetWorld()->GetTimerManager().IsTimerActive(SpinUpTimerHandle))
    {
        return; //already spinning up
    }

    //Replicate
    if (GetNetMode() == NM_Client && UUR_FunctionLibrary::IsComponentLocallyControlled(this))
    {
        ServerSpinUp();
    }
    else if (GetOwnerRole() == ROLE_Authority)
    {
        // Replicate to remotes
        bIsSpinningUpRep = true;
    }

    float CurrentSpinValue = GetCurrentSpinUpValue();

    GetWorld()->GetTimerManager().ClearTimer(SpinDownTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(SpinDownIdleTimerHandle);

    SetBusy(true);

    if (BaseInterface)
    {
        IUR_FireModeBaseInterface::Execute_BeginSpinUp(BaseInterface.GetObject(), this, CurrentSpinValue);
    }

    float Delay = SpinUpTime * (1.f - CurrentSpinValue);
    if (Delay > 0.f)
    {
        GetWorld()->GetTimerManager().SetTimer(SpinUpTimerHandle, this, &UUR_FireModeBase::SpinUpCallback, Delay, false);
    }
    else
    {
        SpinUpCallback();
    }
}

void UUR_FireModeBase::SpinUpCallback()
{
    bFullySpinnedUp = true;

    if (BaseInterface)
    {
        IUR_FireModeBaseInterface::Execute_SpinDone(BaseInterface.GetObject(), this, true);
    }

    StartFire();
}


//============================================================
// StopFire / SpinDown
//============================================================

void UUR_FireModeBase::StopFire_Implementation()
{
    bRequestedFire = false;

    if (SpinUpTime > 0.f || SpinDownTime > 0.f)
    {
        SpinDown();
    }
}

void UUR_FireModeBase::SpinDown()
{
    if (!bFullySpinnedUp && !GetWorld()->GetTimerManager().IsTimerActive(SpinUpTimerHandle))
    {
        return; //already spinning down
    }

    //Replicate
    if (GetNetMode() == NM_Client && UUR_FunctionLibrary::IsComponentLocallyControlled(this))
    {
        ServerSpinDown();
    }
    else if (GetOwnerRole() == ROLE_Authority)
    {
        // Replicate to remotes
        bIsSpinningUpRep = false;
    }

    float CurrentSpinValue = GetCurrentSpinUpValue();

    GetWorld()->GetTimerManager().ClearTimer(SpinUpTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(DelayedSpinUpTimerHandle);

    if (BaseInterface)
    {
        IUR_FireModeBaseInterface::Execute_BeginSpinDown(BaseInterface.GetObject(), this, CurrentSpinValue);
    }

    bFullySpinnedUp = false;

    float IdleDelay = FMath::Max(0.f, SpinDownTime * (CurrentSpinValue - IdleAtSpinPercent));
    if (IdleDelay > 0.f)
    {
        GetWorld()->GetTimerManager().SetTimer(SpinDownIdleTimerHandle, this, &UUR_FireModeBase::SpinDownIdleCallback, IdleDelay, false);
    }
    else
    {
        SpinDownIdleCallback();
    }

    float SpinDownDelay = SpinDownTime * CurrentSpinValue;
    if (SpinDownDelay > 0.f)
    {
        GetWorld()->GetTimerManager().SetTimer(SpinDownTimerHandle, this, &UUR_FireModeBase::SpinDownCallback, SpinDownDelay, false);
    }
    else
    {
        SpinDownCallback();
    }
}

void UUR_FireModeBase::SpinDownIdleCallback()
{
    if (GetTimeUntilIdle() <= 0.f)  //there might be other cooldowns in progress (FireModeBasic)
    {
        SetBusy(false);
    }
}

void UUR_FireModeBase::SpinDownCallback()
{
    if (BaseInterface)
    {
        IUR_FireModeBaseInterface::Execute_SpinDone(BaseInterface.GetObject(), this, false);
    }
}


//============================================================
// Replication
//============================================================

void UUR_FireModeBase::ServerSpinUp_Implementation()
{
    // Validation
    if (BaseInterface)
    {
        float Delay = IUR_FireModeBaseInterface::Execute_TimeUntilReadyToFire(BaseInterface.GetObject(), this);
        if (Delay > 0.f)
        {
            UE_LOG(LogWeapon, Log, TEXT("ServerSpinUp Delay = %f"), Delay);
            if (Delay < TIMEUNTILFIRE_NEVER)
            {
                GetWorld()->GetTimerManager().SetTimer(DelayedSpinUpTimerHandle, this, &UUR_FireModeBase::ServerSpinUp_Implementation, Delay, false);
            }
            return;
        }
    }

    SpinUp();
}

void UUR_FireModeBase::ServerSpinDown_Implementation()
{
    SpinDown();
}

void UUR_FireModeBase::OnRep_IsSpinningUp()
{
    if (GetNetMode() == NM_Client)
    {
        if (bIsSpinningUpRep)
        {
            SpinUp();
        }
        else
        {
            SpinDown();
        }
    }
}


//============================================================
// Utilities
//============================================================

UUR_FireModeBase::UUR_FireModeBase()
    : Index(0)
    , SpinUpTime(0.f)
    , SpinDownTime(0.f)
    , IdleAtSpinPercent(1.f)
{
    //UActorComponent::SetAutoActivate(false);
    bAutoActivate = false;
    bWasActive = false;

    PrimaryComponentTick.bStartWithTickEnabled = false;
    bAutoActivateTick = false;

    SetIsReplicatedByDefault(true);

    InitialAmmoCost = 1;
    MuzzleSocketName = FName(TEXT("Muzzle"));
    MuzzleFlashScale = 1.f;
}

float UUR_FireModeBase::GetCurrentSpinUpValue()
{
    if (bFullySpinnedUp)
    {
        return 1.f;
    }
    if (GetWorld()->GetTimerManager().IsTimerActive(SpinUpTimerHandle))
    {
        return 1.f - GetWorld()->GetTimerManager().GetTimerRemaining(SpinUpTimerHandle) / SpinUpTime;
    }
    if (GetWorld()->GetTimerManager().IsTimerActive(SpinDownTimerHandle))
    {
        return GetWorld()->GetTimerManager().GetTimerRemaining(SpinDownTimerHandle) / SpinDownTime;
    }
    return 0.f;
}

float UUR_FireModeBase::GetTimeUntilIdle_Implementation()
{
    if (IdleAtSpinPercent < 1.f)
    {
        return FMath::Max(0.f, SpinDownTime * (GetCurrentSpinUpValue() - IdleAtSpinPercent));
    }
    return 0.f;
}

float UUR_FireModeBase::GetCooldownStartTime_Implementation()
{
    if (GetWorld()->GetTimerManager().TimerExists(SpinDownIdleTimerHandle))
    {
        //return GetWorld()->GetTimeSeconds() - GetWorld()->GetTimerManager().GetTimerElapsed(SpinDownIdleTimerHandle);

        /** WARNING: we can spindown in the middle of a spinup.
        *
        * Eg: spinup to 0.6, and start spinning down.
        * With IdleAtSpinPercent 0.5, the firemode becomes idle after a 0.1s timer only.
        * Because of this, GetTimerElapsed() is not going to give us consistent results.
        *
        * This function is used in conjunction with GetTimeUntilIdle() to figure out weapon swap times.
        * We want swap times to be consistent, therefore the time at which we can swap during spindown must be consistent.
        *
        * To get consistent results we need to compute from the total spin down time (1.0 -> 0.0),
        * regardless of when we actually started to spindown.
        */
        float Remaining = GetWorld()->GetTimerManager().GetTimerRemaining(SpinDownIdleTimerHandle);
        float Total = SpinDownTime * (1.f - IdleAtSpinPercent);
        float Elapsed = Total - Remaining;
        return GetWorld()->GetTimeSeconds() - Elapsed;
    }
    return 0.f;
}


//============================================================
// UActorComponent tweaks
//============================================================

void UUR_FireModeBase::Activate(bool bReset)
{
    if (bReset || ShouldActivate())
    {
        SetActiveFlag(true);
        if (bAutoActivateTick || PrimaryComponentTick.bStartWithTickEnabled)
        {
            // Now tick only activates when it is configured to
            SetComponentTickEnabled(true);
        }
        OnComponentActivated.Broadcast(this, bReset);
    }
    bWasActive = true;
}

void UUR_FireModeBase::Deactivate()
{
    StopFire();

    Super::Deactivate();

    bWasActive = false;
}

void UUR_FireModeBase::OnRep_IsActive()
{
    bool bNewActive = IsActive();
    if (bNewActive != bWasActive)
    {
        SetActiveFlag(bWasActive);
        SetActive(bNewActive);
    }
}
