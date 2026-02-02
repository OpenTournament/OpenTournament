// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <AbilitySystemInterface.h>
#include <GameplayTagContainer.h>
#include <ModularGameState.h>

#include "UR_GameState.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

#define UE_API OPENTOURNAMENT_API

class UUR_AbilitySystemComponent;
class UUR_ExperienceManagerComponent;
class AUR_PlayerState;
class AUR_TeamInfo;
class AUR_Pickup;

/////////////////////////////////////////////////////////////////////////////////////////////////
// Delegates

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMatchStateChanged, AUR_GameState*, GS);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTimeUpSignature, AUR_GameState*, GS);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FFragEventSignature, AUR_PlayerState*, Victim, AUR_PlayerState*, Killer, TSubclassOf<UDamageType>, DamType, const FGameplayTagContainer&, EventTags);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGlobalPickupEventSignature, TSubclassOf<AUR_Pickup>, PickupClass, AUR_PlayerState*, Recipient);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWinnerAssignedSignature, AUR_GameState*, GS);

/////////////////////////////////////////////////////////////////////////////////////////////////

// @! TODO : Extend AModulerGameStateBase not AModularGameState
/**
 *
 */
UCLASS()
class AUR_GameState
    : public AModularGameState
    , public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    UE_API AUR_GameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    //~IAbilitySystemInterface
    UE_API virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    //~End of IAbilitySystemInterface

    // Gets the ability system component used for game wide things
    UFUNCTION(BlueprintCallable, Category = "OT|GameState")
    UUR_AbilitySystemComponent* GetGameAbilitySystemComponent() const { return AbilitySystemComponent; }

    // Gets the server's FPS, replicated to clients
    UE_API float GetServerFPS() const;

protected:
    UPROPERTY(Replicated)
    float ServerFPS;


    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Match state
    /////////////////////////////////////////////////////////////////////////////////////////////////

public:
    UPROPERTY(BlueprintAssignable)
    FMatchStateChanged OnMatchStateChanged;

    UPROPERTY(BlueprintAssignable)
    FMatchStateChanged OnMatchStateTagChanged;

    UFUNCTION(BlueprintPure)
    FGameplayTag GetMatchStateTag() const
    {
        return MatchStateTag;
    }

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
    void SetMatchStateTag(const FGameplayTag& NewTag);

protected:
    virtual void OnRep_MatchState() override;

    /**
    * GameplayTag defining a sub-state for the current match state.
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
    UPROPERTY(ReplicatedUsing = OnRep_MatchStateTag)
    FGameplayTag MatchStateTag;

    UFUNCTION()
    virtual void OnRep_MatchStateTag();

    /**
    * When a match state repeats itself, MatchStateTag would not replicate again (because it doesn't change).
    * This is automatically detected (in SetMatchStateTag) and Multicast is used instead to force replication & RepNotify events.
    *
    * Use case: When entering a second OVERTIME, we want to notify players with a second OVERTIME game event.
    */
    UFUNCTION(NetMulticast, Reliable)
    void MulticastMatchStateTag(const FGameplayTag& NewTag);

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

        RemainingTime = FMath::Max(0, TimeLimit);
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
    UPROPERTY(BlueprintAssignable)
    FFragEventSignature FragEvent;

    // @! TODO : Use GameplayMessageSubsystem
    UFUNCTION(NetMulticast, Reliable)
    void MulticastFragEvent(AUR_PlayerState* Victim, AUR_PlayerState* Killer, TSubclassOf<UDamageType> DamType, const FGameplayTagContainer& EventTags);

    virtual void MulticastFragEvent_Implementation(AUR_PlayerState* Victim, AUR_PlayerState* Killer, TSubclassOf<UDamageType> DamType, const FGameplayTagContainer& EventTags)
    {
        FragEvent.Broadcast(Victim, Killer, DamType, EventTags);
    }

    /**
    * Global pickup event for major items.
    * We pass a PickupClass here because the pickup may not always be relevant to players.
    */
    UPROPERTY(BlueprintAssignable)
    FGlobalPickupEventSignature PickupEvent;

    // @! TODO : Use GameplayMessageSubsystem
    UFUNCTION(NetMulticast, Reliable)
    void MulticastPickupEvent(TSubclassOf<AUR_Pickup> PickupClass, AUR_PlayerState* Recipient);

    virtual void MulticastPickupEvent_Implementation(TSubclassOf<AUR_Pickup> PickupClass, AUR_PlayerState* Recipient)
    {
        PickupEvent.Broadcast(PickupClass, Recipient);
    }

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

    ////

    // The ability system component subobject for game-wide things (primarily gameplay cues)
    UPROPERTY(VisibleAnywhere, Category = "GameState")
    TObjectPtr<UUR_AbilitySystemComponent> AbilitySystemComponent;

    // Handles loading and managing the current gameplay experience
    UPROPERTY()
    TObjectPtr<UUR_ExperienceManagerComponent> ExperienceManagerComponent;
};

#undef UE_API
