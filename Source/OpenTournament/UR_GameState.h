// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"

#include "UR_GameState.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AUR_TeamInfo;
class AUR_Pickup;

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace FragEventExtras
{
    //NOTE: Multis and Sprees sit in TArray properties to be easily moddable

    static const FName SpreeEnded = FName(TEXT("SpreeEnded"));

    static const FName AmazingCombo = FName(TEXT("AmazingCombo"));
    static const FName AirRocket = FName(TEXT("AirRocket"));

    static const FName Revenge = FName(TEXT("Revenge"));
};

/////////////////////////////////////////////////////////////////////////////////////////////////
// Delegates

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMatchStateChanged, AUR_GameState*, GS);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTimeUpSignature, AUR_GameState*, GS);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FFragEventSignature, AUR_PlayerState*, Victim, AUR_PlayerState*, Killer, TSubclassOf<UDamageType>, DamType, const TArray<FName>&, Extras);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPickupEventSignature, AUR_Pickup*, Pickup, AUR_PlayerState*, Recipient);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWinnerAssignedSignature, AUR_GameState*, GS);

/////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * 
 */
UCLASS()
class OPENTOURNAMENT_API AUR_GameState : public AGameState
{
    GENERATED_BODY()

protected:

    AUR_GameState();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Match state
    /////////////////////////////////////////////////////////////////////////////////////////////////

public:

    UPROPERTY(BlueprintAssignable)
    FMatchStateChanged OnMatchStateChanged;

    UPROPERTY(BlueprintAssignable)
    FMatchStateChanged OnMatchSubStateChanged;

    UFUNCTION(BlueprintPure)
    FName GetMatchSubState() const { return MatchSubState; }

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
    void SetMatchSubState(FName NewState);

protected:

    virtual void OnRep_MatchState() override;

    /**
    * Sub-state for the InProgress match state.
    *
    * We don't want to mess with the existing framework provided for MatchState.
    * GameMode.h specifically states the following :
    * MatchState::InProgress = Normal gameplay is occurring. Specific games will have their own state machine inside this state
    *
    * Within the InProgress state we can implement possible sub-states such as :
    * - Warmup
    * - Countdown
    * - First Half
    * - Intermission
    * - Second Half
    * - Overtime
    */
    UPROPERTY(ReplicatedUsing = OnRep_MatchSubState)
    FName MatchSubState;

    UFUNCTION()
    virtual void OnRep_MatchSubState();

    /**
    * When a match state repeats itself, MatchSubState would not replicate again (because it doesn't change).
    * This is automatically detected (in SetMatchSubState) and Multicast is used instead to force replication & RepNotify events.
    *
    * Use case: When entering a second OVERTIME, we want to notify players with a second OVERTIME game event.
    */
    UFUNCTION(NetMulticast, Reliable)
    void MulticastMatchSubState(FName NewState);

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Clock Management
    /////////////////////////////////////////////////////////////////////////////////////////////////

protected:

    /**
    * Engine framework provides ElapsedTime, with replication InitialOnly.
    * We need to sync up the clock every now and then, otherwise it will derive.
    */
    UFUNCTION(NetMulticast, Reliable)
    void MulticastElapsedTime(float ServerElapsedTime);
    virtual void MulticastElapsedTime_Implementation(float ServerElapsedTime)
    {
        ElapsedTime = ServerElapsedTime;
    }

    /**
    * Now we can use ElapsedTime as an internal clock.
    * Most of the time however, we are more interested in remaining time from a time limit.
    */

public:

    /**
    * Current match/round/stage time limit.
    * Use 0 for no time limit.
    */
    UPROPERTY(Replicated, BlueprintReadOnly)
    int32 TimeLimit;

    /**
    * Reference point to calculate the current match/round/stage elapsed time or remaining time.
    */
    UPROPERTY(ReplicatedUsing = OnRep_ClockReferencePoint)
    int32 ClockReferencePoint;

    /**
    * Calculated remaining time based on the current reference point, assuming TimeLimit is set.
    */
    UPROPERTY(BlueprintReadOnly)
    int32 RemainingTime;

    /**
    * Define a new time limit and update the clock.
    */
    UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
    virtual void SetTimeLimit(int32 NewTimeLimit)
    {
        TimeLimit = NewTimeLimit;
        ClockReferencePoint = ElapsedTime;
        ForceNetUpdate();

        bTriggeredTimeUp = false;
    }

    /**
    * When there is no time limit, use this to restart the clock.
    */
    UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
    virtual void ResetClock()
    {
        TimeLimit = 0;
        ClockReferencePoint = ElapsedTime;
        ForceNetUpdate();
    }

    /**
    * Get elapsed time according to the last clock reset.
    */
    UFUNCTION(BlueprintCallable, BlueprintPure)
    virtual int32 GetCurrentElapsedTime()
    {
        return ElapsedTime - ClockReferencePoint;
    }

    virtual void DefaultTimer() override;

    /**
    * Flag to remember if we already triggered TimeUp() in the current stage.
    */
    UPROPERTY()
    bool bTriggeredTimeUp;

    /**
    * Triggered when RemainingTime reaches 0.
    * Only applicable with a set TimeLimit.
    * Triggers only once per "stage". Will trigger only after calling SetTimeLimit again.
    *
    * NOTE: Can trigger on client, but might not be reliable if server sets a new timelimit.
    * You are better off multicasting from server if needed.
    */
    UPROPERTY(BlueprintAssignable)
    FTimeUpSignature OnTimeUp;

protected:

    UFUNCTION()
    virtual void OnRep_ClockReferencePoint()
    {
        bTriggeredTimeUp = false;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Teams
    /////////////////////////////////////////////////////////////////////////////////////////////////

public:

    /** Array of TeamInfos, maintained on both server and clients (TeamInfos are always relevant) */
    UPROPERTY(BlueprintReadOnly)
    TArray<AUR_TeamInfo*> Teams;

    /**
    * Create and register a new Team.
    */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
    virtual AUR_TeamInfo* AddNewTeam();

    /**
    * Remove trailing empty teams, keeping a minimum of 2 teams.
    */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
    virtual void TrimTeams();

    /**
    * Utility - Get all PlayerStates with bOnlySpectator = true.
    */
    UFUNCTION(BlueprintCallable)
    virtual void GetSpectators(TArray<APlayerState*>& OutSpectators);

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Game Events
    /////////////////////////////////////////////////////////////////////////////////////////////////

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> MultiKillEventNames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> SpreeEventNames;

    UPROPERTY(BlueprintAssignable)
    FFragEventSignature FragEvent;

    UFUNCTION(NetMulticast, Reliable)
    void MulticastFragEvent(AUR_PlayerState* Victim, AUR_PlayerState* Killer, TSubclassOf<UDamageType> DamType, const TArray<FName>& Events);
    virtual void MulticastFragEvent_Implementation(AUR_PlayerState* Victim, AUR_PlayerState* Killer, TSubclassOf<UDamageType> DamType, const TArray<FName>& Extras)
    {
        FragEvent.Broadcast(Victim, Killer, DamType, Extras);
    }

    UPROPERTY(BlueprintAssignable)
    FPickupEventSignature PickupEvent;

    /**
    * NOTE: Not really a multicast for security reasons.
    * Replication is handled manually.
    */
    UFUNCTION(BlueprintAuthorityOnly)
    virtual void MulticastPickupEvent(AUR_Pickup* Pickup, AUR_PlayerState* Recipient);

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // End Game
    /////////////////////////////////////////////////////////////////////////////////////////////////

public:

    UPROPERTY(ReplicatedUsing = OnRep_Winner, BlueprintReadOnly)
    AActor* Winner;

    UPROPERTY(BlueprintReadOnly)
    AActor* EndGameFocus;

    UFUNCTION()
    virtual void OnRep_Winner();

    UPROPERTY(BlueprintAssignable)
    FWinnerAssignedSignature OnWinnerAssigned;
};
