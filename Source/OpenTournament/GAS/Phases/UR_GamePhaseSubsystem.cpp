// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "GAS/Phases/UR_GamePhaseSubsystem.h"

#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"

#include "UR_GamePhaseAbility.h"
#include "UR_GamePhaseLog.h"
#include "GAS/UR_AbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_GamePhaseSubsystem)

/////////////////////////////////////////////////////////////////////////////////////////////////

class UUR_GameplayAbility;
class UObject;

/////////////////////////////////////////////////////////////////////////////////////////////////

DEFINE_LOG_CATEGORY(LogGamePhase);

/////////////////////////////////////////////////////////////////////////////////////////////////
// UUR_GamePhaseSubsystem

UUR_GamePhaseSubsystem::UUR_GamePhaseSubsystem()
{}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_GamePhaseSubsystem::PostInitialize()
{
    Super::PostInitialize();
}

bool UUR_GamePhaseSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (Super::ShouldCreateSubsystem(Outer))
    {
        //UWorld* World = Cast<UWorld>(Outer);
        //check(World);

        //return World->GetAuthGameMode() != nullptr;
        //return nullptr;
        return true;
    }

    return false;
}

bool UUR_GamePhaseSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
    return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UUR_GamePhaseSubsystem::StartPhase(TSubclassOf<UUR_GamePhaseAbility> PhaseAbility, FGamePhaseDelegate PhaseEndedCallback)
{
    UWorld* World = GetWorld();
    UUR_AbilitySystemComponent* GameState_ASC = World->GetGameState()->FindComponentByClass<UUR_AbilitySystemComponent>();
    if (ensure(GameState_ASC))
    {
        FGameplayAbilitySpec PhaseSpec(PhaseAbility, 1, 0, this);
        FGameplayAbilitySpecHandle SpecHandle = GameState_ASC->GiveAbilityAndActivateOnce(PhaseSpec);
        FGameplayAbilitySpec* FoundSpec = GameState_ASC->FindAbilitySpecFromHandle(SpecHandle);

        if (FoundSpec && FoundSpec->IsActive())
        {
            FGamePhaseEntry& Entry = ActivePhaseMap.FindOrAdd(SpecHandle);
            Entry.PhaseEndedCallback = PhaseEndedCallback;
        }
        else
        {
            PhaseEndedCallback.ExecuteIfBound(nullptr);
        }
    }
}

void UUR_GamePhaseSubsystem::K2_StartPhase(TSubclassOf<UUR_GamePhaseAbility> PhaseAbility, const FGamePhaseDynamicDelegate& PhaseEndedDelegate)
{
    const FGamePhaseDelegate EndedDelegate = FGamePhaseDelegate::CreateWeakLambda
    (const_cast<UObject*>(PhaseEndedDelegate.GetUObject()),
        [PhaseEndedDelegate](const UUR_GamePhaseAbility* PhaseAbility)
        {
            PhaseEndedDelegate.ExecuteIfBound(PhaseAbility);
        });

    StartPhase(PhaseAbility, EndedDelegate);
}

void UUR_GamePhaseSubsystem::K2_WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FGamePhaseTagDynamicDelegate WhenPhaseActive)
{
    const FGamePhaseTagDelegate ActiveDelegate = FGamePhaseTagDelegate::CreateWeakLambda
    (WhenPhaseActive.GetUObject(),
        [WhenPhaseActive](const FGameplayTag& InPhaseTag)
        {
            WhenPhaseActive.ExecuteIfBound(InPhaseTag);
        });

    WhenPhaseStartsOrIsActive(PhaseTag, MatchType, ActiveDelegate);
}

void UUR_GamePhaseSubsystem::K2_WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FGamePhaseTagDynamicDelegate WhenPhaseEnd)
{
    const FGamePhaseTagDelegate EndedDelegate = FGamePhaseTagDelegate::CreateWeakLambda
    (WhenPhaseEnd.GetUObject(),
        [WhenPhaseEnd](const FGameplayTag& InPhaseTag)
        {
            WhenPhaseEnd.ExecuteIfBound(InPhaseTag);
        });

    WhenPhaseEnds(PhaseTag, MatchType, EndedDelegate);
}

void UUR_GamePhaseSubsystem::WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FGamePhaseTagDelegate& WhenPhaseActive)
{
    FPhaseObserver Observer;
    Observer.PhaseTag = PhaseTag;
    Observer.MatchType = MatchType;
    Observer.PhaseCallback = WhenPhaseActive;
    PhaseStartObservers.Add(Observer);

    if (IsPhaseActive(PhaseTag))
    {
        WhenPhaseActive.ExecuteIfBound(PhaseTag);
    }
}

void UUR_GamePhaseSubsystem::WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FGamePhaseTagDelegate& WhenPhaseEnd)
{
    FPhaseObserver Observer;
    Observer.PhaseTag = PhaseTag;
    Observer.MatchType = MatchType;
    Observer.PhaseCallback = WhenPhaseEnd;
    PhaseEndObservers.Add(Observer);
}

bool UUR_GamePhaseSubsystem::IsPhaseActive(const FGameplayTag& PhaseTag) const
{
    for (const auto& KVP : ActivePhaseMap)
    {
        const FGamePhaseEntry& PhaseEntry = KVP.Value;
        if (PhaseEntry.PhaseTag.MatchesTag(PhaseTag))
        {
            return true;
        }
    }

    return false;
}

void UUR_GamePhaseSubsystem::OnBeginPhase(const UUR_GamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle)
{
    const FGameplayTag IncomingPhaseTag = PhaseAbility->GetGamePhaseTag();

    UE_LOG(LogGamePhase, Log, TEXT("Beginning Phase '%s' (%s)"), *IncomingPhaseTag.ToString(), *GetNameSafe(PhaseAbility));

    const UWorld* World = GetWorld();
    UUR_AbilitySystemComponent* GameState_ASC = World->GetGameState()->FindComponentByClass<UUR_AbilitySystemComponent>();
    if (ensure(GameState_ASC))
    {
        TArray<FGameplayAbilitySpec*> ActivePhases;
        for (const auto& KVP : ActivePhaseMap)
        {
            const FGameplayAbilitySpecHandle ActiveAbilityHandle = KVP.Key;
            if (FGameplayAbilitySpec* Spec = GameState_ASC->FindAbilitySpecFromHandle(ActiveAbilityHandle))
            {
                ActivePhases.Add(Spec);
            }
        }

        for (const FGameplayAbilitySpec* ActivePhase : ActivePhases)
        {
            const UUR_GamePhaseAbility* ActivePhaseAbility = CastChecked<UUR_GamePhaseAbility>(ActivePhase->Ability);
            const FGameplayTag ActivePhaseTag = ActivePhaseAbility->GetGamePhaseTag();

            // So if the active phase currently matches the incoming phase tag, we allow it.
            // i.e. multiple gameplay abilities can all be associated with the same phase tag.
            // For example,
            // You can be in the, Game.Playing, phase, and then start a sub-phase, like Game.Playing.SuddenDeath
            // Game.Playing phase will still be active, and if someone were to push another one, like,
            // Game.Playing.ActualSuddenDeath, it would end Game.Playing.SuddenDeath phase, but Game.Playing would
            // continue.  Similarly if we activated Game.GameOver, all the Game.Playing* phases would end.
            if (!IncomingPhaseTag.MatchesTag(ActivePhaseTag))
            {
                UE_LOG(LogGamePhase, Log, TEXT("\tEnding Phase '%s' (%s)"), *ActivePhaseTag.ToString(), *GetNameSafe(ActivePhaseAbility));

                FGameplayAbilitySpecHandle HandleToEnd = ActivePhase->Handle;
                GameState_ASC->CancelAbilitiesByFunc
                ([HandleToEnd]
                (
                    const UUR_GameplayAbility* InAbility, FGameplayAbilitySpecHandle Handle
                )
                    {
                        return Handle == HandleToEnd;
                    },
                    true);
            }
        }

        FGamePhaseEntry& Entry = ActivePhaseMap.FindOrAdd(PhaseAbilityHandle);
        Entry.PhaseTag = IncomingPhaseTag;

        // Notify all observers of this phase that it has started.
        for (const FPhaseObserver& Observer : PhaseStartObservers)
        {
            if (Observer.IsMatch(IncomingPhaseTag))
            {
                Observer.PhaseCallback.ExecuteIfBound(IncomingPhaseTag);
            }
        }
    }
}

void UUR_GamePhaseSubsystem::OnEndPhase(const UUR_GamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle)
{
    const FGameplayTag EndedPhaseTag = PhaseAbility->GetGamePhaseTag();
    UE_LOG(LogGamePhase, Log, TEXT("Ended Phase '%s' (%s)"), *EndedPhaseTag.ToString(), *GetNameSafe(PhaseAbility));

    const FGamePhaseEntry& Entry = ActivePhaseMap.FindChecked(PhaseAbilityHandle);
    Entry.PhaseEndedCallback.ExecuteIfBound(PhaseAbility);

    ActivePhaseMap.Remove(PhaseAbilityHandle);

    // Notify all observers of this phase that it has ended.
    for (const FPhaseObserver& Observer : PhaseEndObservers)
    {
        if (Observer.IsMatch(EndedPhaseTag))
        {
            Observer.PhaseCallback.ExecuteIfBound(EndedPhaseTag);
        }
    }
}

bool UUR_GamePhaseSubsystem::FPhaseObserver::IsMatch(const FGameplayTag& ComparePhaseTag) const
{
    switch (MatchType)
    {
        case EPhaseTagMatchType::ExactMatch:
        {
            return ComparePhaseTag == PhaseTag;
        }
        case EPhaseTagMatchType::PartialMatch:
        {
            return ComparePhaseTag.MatchesTag(PhaseTag);
        }
        default:
        {
            return false;
        }
    }
}
