// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_TeamSpawningManagerComponent.h"

#include <GameFramework/PlayerState.h>

#include "GameModes/UR_GameState.h"
#include "Player/UR_PlayerStart.h"
#include "Teams/UR_TeamSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_TeamSpawningManagerComponent)

/////////////////////////////////////////////////////////////////////////////////////////////////

class AActor;

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_TeamSpawningManagerComponent::UUR_TeamSpawningManagerComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Noop
}

/////////////////////////////////////////////////////////////////////////////////////////////////

AActor* UUR_TeamSpawningManagerComponent::OnChoosePlayerStart(AController* Player, TArray<AUR_PlayerStart*>& PlayerStarts)
{
    UUR_TeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<UUR_TeamSubsystem>();
    if (!ensure(TeamSubsystem))
    {
        return nullptr;
    }

    const int32 PlayerTeamId = TeamSubsystem->FindTeamFromObject(Player);

    // We should have a TeamId by now, but early login stuff before post login can try to do stuff, ignore it.
    if (!ensure(PlayerTeamId != INDEX_NONE))
    {
        return nullptr;
    }

    AUR_GameState* GameState = GetGameStateChecked<AUR_GameState>();

    AUR_PlayerStart* BestPlayerStart = nullptr;
    double MaxDistance = 0;
    AUR_PlayerStart* FallbackPlayerStart = nullptr;
    double FallbackMaxDistance = 0;

    for (const APlayerState* PS : GameState->PlayerArray)
    {
        const int32 TeamId = TeamSubsystem->FindTeamFromObject(PS);

        // We should have a TeamId by now...
        if (PS->IsOnlyASpectator() || !ensure(TeamId != INDEX_NONE))
        {
            continue;
        }

        // If the other player isn't on the same team, lets find the furthest spawn from them.
        if (TeamId != PlayerTeamId)
        {
            for (AUR_PlayerStart* PlayerStart : PlayerStarts)
            {
                if (const APawn* Pawn = PS->GetPawn())
                {
                    const double Distance = PlayerStart->GetDistanceTo(Pawn);

                    if (PlayerStart->IsClaimed())
                    {
                        if (FallbackPlayerStart == nullptr || Distance > FallbackMaxDistance)
                        {
                            FallbackPlayerStart = PlayerStart;
                            FallbackMaxDistance = Distance;
                        }
                    }
                    else if (PlayerStart->GetLocationOccupancy(Player) < EGamePlayerStartLocationOccupancy::Full)
                    {
                        if (BestPlayerStart == nullptr || Distance > MaxDistance)
                        {
                            BestPlayerStart = PlayerStart;
                            MaxDistance = Distance;
                        }
                    }
                }
            }
        }
    }

    if (BestPlayerStart)
    {
        return BestPlayerStart;
    }

    return FallbackPlayerStart;
}

void UUR_TeamSpawningManagerComponent::OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation)
{
    // Noop
}
