// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

#include "UR_FireModeContinuous.h"

#include "UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"

#include "UR_FunctionLibrary.h"

void UUR_FireModeContinuous::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(UUR_FireModeContinuous, bIsFiringRep, COND_SkipOwner);
}

void UUR_FireModeContinuous::StartFire_Implementation()
{
    SetBusy(true);

    if (ContinuousInterface)
    {
        IUR_FireModeContinuousInterface::Execute_StartContinuousEffects(ContinuousInterface.GetObject(), this);
    }

    DeltaTimeAccumulator = 0.f;
    SetComponentTickEnabled(true);

    if (UUR_FunctionLibrary::IsComponentLocallyControlled(this))
    {
        ServerStartFire();
    }
}

void UUR_FireModeContinuous::StopFire_Implementation()
{
    SetComponentTickEnabled(false);

    if (ContinuousInterface)
    {
        IUR_FireModeContinuousInterface::Execute_StopContinuousEffects(ContinuousInterface.GetObject(), this);
    }

    //NOTE: not sure if we should wait next tick
    SetBusy(false);

    if (UUR_FunctionLibrary::IsComponentLocallyControlled(this))
    {
        ServerStopFire();
    }
}

float UUR_FireModeContinuous::GetTimeUntilIdle_Implementation()
{
    if (bIsBusy)
    {
        return 0.001f;
    }
    return 0.f;
}

void UUR_FireModeContinuous::ServerStartFire_Implementation()
{
    if (BaseInterface)
    {
        // Continuous modes are essentially cooldown-less (until we implement spinup/spindown)
        // But a delay might come in from a sibling firemode in the weapon.
        float Delay = IUR_FireModeBaseInterface::Execute_TimeUntilReadyToFire(BaseInterface.GetObject(), this);
        if (Delay > 0.f)
        {
            GetWorld()->GetTimerManager().SetTimer(DelayedStartFireTimerHandle, this, &UUR_FireModeContinuous::ServerStartFire_Implementation, Delay, false);
            return;
        }
    }

    SetBusy(true);
    DeltaTimeAccumulator = 0.f;
    SetComponentTickEnabled(true);

    // Replicate to remotes
    bIsFiringRep = true;

    if (ContinuousInterface)
    {
        IUR_FireModeContinuousInterface::Execute_AuthorityStartContinuousFire(ContinuousInterface.GetObject(), this);
    }
}

void UUR_FireModeContinuous::ServerStopFire_Implementation()
{
    GetWorld()->GetTimerManager().ClearTimer(DelayedStartFireTimerHandle);

    SetComponentTickEnabled(false);
    SetBusy(false);

    // Replicate to remotes
    bIsFiringRep = false;

    if (ContinuousInterface)
    {
        IUR_FireModeContinuousInterface::Execute_AuthorityStopContinuousFire(ContinuousInterface.GetObject(), this);
    }
}

void UUR_FireModeContinuous::OnRep_IsFiring()
{
    if (GetNetMode() == NM_Client)
    {
        if (bIsFiringRep)
        {
            StartFire();
        }
        else
        {
            StopFire();
        }
    }
}

void UUR_FireModeContinuous::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    // Hit check first, on controlling client or authority
    if (UUR_FunctionLibrary::IsComponentLocallyControlled(this) || GetOwnerRole() == ROLE_Authority)
    {
        DeltaTimeAccumulator += DeltaTime;

        //NOTE: allow a minor negative margin to improve the accuracy of intervals
        if (DeltaTimeAccumulator >= HitCheckInterval - 0.2f*DeltaTime)
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
}
