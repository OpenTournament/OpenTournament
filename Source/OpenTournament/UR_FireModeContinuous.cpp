// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_FireModeContinuous.h"

#include "Engine/World.h"
#include "TimerManager.h"

#include "UR_FunctionLibrary.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
* NOTE:
* The spinup routine already has replication mechanisms which are exactly what we want.
* We are going to rest upon it even when we don't have any spinup.
* The cases SpinUpTime=0/SpinDownTime=0 are handled and will simply skip all delays.
*
* The only downside is that interface's SpinUp callbacks are going to be called even if there is no spinup.
*/

void UUR_FireModeContinuous::RequestStartFire_Implementation()
{
    bRequestedFire = true;
    SpinUp();
}

// Called after the spinup routine, on clients & server
void UUR_FireModeContinuous::StartFire_Implementation()
{
    DeltaTimeAccumulator = 0.f;
    SetComponentTickEnabled(true);

    if (ContinuousInterface)
    {
        if (GetNetMode() != NM_DedicatedServer)
        {
            IUR_FireModeContinuousInterface::Execute_StartContinuousEffects(ContinuousInterface.GetObject(), this);
        }
        if (GetOwnerRole() == ROLE_Authority)
        {
            IUR_FireModeContinuousInterface::Execute_AuthorityStartContinuousFire(ContinuousInterface.GetObject(), this);
        }
    }
}

void UUR_FireModeContinuous::StopFire_Implementation()
{
    bRequestedFire = false;
    SpinDown();
}

void UUR_FireModeContinuous::SpinDown()
{
    SetComponentTickEnabled(false);

    if (ContinuousInterface)
    {
        if (GetNetMode() != NM_DedicatedServer)
        {
            IUR_FireModeContinuousInterface::Execute_StopContinuousEffects(ContinuousInterface.GetObject(), this);
        }
        if (GetOwnerRole() == ROLE_Authority)
        {
            IUR_FireModeContinuousInterface::Execute_AuthorityStopContinuousFire(ContinuousInterface.GetObject(), this);
        }
    }

    Super::SpinDown();
}

float UUR_FireModeContinuous::GetTimeUntilIdle_Implementation()
{
    if (bIsBusy)
    {
        // if idle is delayed by spindown, return that
        if (SpinDownTime > 0.f && IdleAtSpinPercent < 1.f)
        {
            return Super::GetTimeUntilIdle_Implementation();
        }
        // if no spindown, return at least 1 frame when firing so a cheater cannot fire two firemodes simultaneously
        if (bFullySpinnedUp)
        {
            return 0.001f;
        }
    }
    return 0.f;
}

void UUR_FireModeContinuous::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    // Hit check first, on controlling client or authority
    if (UUR_FunctionLibrary::IsComponentLocallyControlled(this) || GetOwnerRole() == ROLE_Authority)
    {
        DeltaTimeAccumulator += DeltaTime;

        //NOTE: allow a minor negative margin to improve the accuracy of intervals
        if (DeltaTimeAccumulator >= HitCheckInterval - 0.2f * DeltaTime)
        {
            if (ContinuousInterface)
            {
                if (UUR_FunctionLibrary::IsComponentLocallyControlled(this))
                {
                    IUR_FireModeContinuousInterface::Execute_SimulateContinuousHitCheck(ContinuousInterface.GetObject(), this);
                }
                if (GetOwnerRole() == ROLE_Authority)
                {
                    IUR_FireModeContinuousInterface::Execute_AuthorityContinuousHitCheck(ContinuousInterface.GetObject(), this);
                }
            }

            // If we had a high frametime hitch, allow up to two ticks in a row to do a hit check, but never more
            DeltaTimeAccumulator = FMath::Min(DeltaTimeAccumulator - HitCheckInterval, HitCheckInterval);
        }
    }

    // Visuals second, on all clients
    if (ContinuousInterface && GetNetMode() != NM_DedicatedServer)
    {
        IUR_FireModeContinuousInterface::Execute_UpdateContinuousEffects(ContinuousInterface.GetObject(), this, DeltaTime);
    }

    // BP Tick
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
