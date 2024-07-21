// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_BotController.h"

#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerState.h"

#include "UR_AIAimComp.h"
#include "UR_AINavigationJumpingComp.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_BotController::AUR_BotController()
{
    bWantsPlayerState = true;

    AimComponent = CreateDefaultSubobject<UUR_AIAimComp>("AimComponent");
    NavigationJumpingComponent = CreateDefaultSubobject<UUR_AINavigationJumpingComp>("NavigationJumpingComponent");

    OnNewPawn.AddUObject(this, &AUR_BotController::OnNewPawnHandler);
}

void AUR_BotController::InitPlayerState()
{
    Super::InitPlayerState();

    if (!IsNetMode(NM_Client))
    {
        if (auto PS = GetPlayerState<APlayerState>())
        {
            PS->SetPlayerNameInternal(FString::Printf(TEXT("OTBot-%d"), FMath::RandRange(10, 9999)));
        }
    }
}

void AUR_BotController::OnNewPawnHandler(APawn* P)
{
    if (P)
    {
        GetWorld()->GetTimerManager().ClearTimer(RespawnTimerHandle);
    }
    else
    {
        //TODO: Obey gamemode's respawn rules
        float Delay = FMath::RandRange(1.f, 4.f);
        //NOTE: We use loop to attempt respawn every second in case respawn fails
        GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, FTimerDelegate::CreateUObject(this, &AUR_BotController::Respawn), 1.f, true, Delay);
    }
}

void AUR_BotController::Respawn()
{
    if (auto GM = GetWorld()->GetAuthGameMode())
    {
        GM->RestartPlayer(this);
    }
}

void AUR_BotController::UpdateControlRotation(float DeltaTime, bool bUpdatePawn)
{
    Super::UpdateControlRotation(DeltaTime, bUpdatePawn);

    if (bUpdatePawn)
    {
        if (auto P = GetPawn())
            P->SetRemoteViewPitch(GetControlRotation().Pitch);
    }
}

FVector AUR_BotController::GetFocalPointOnActor(const AActor* Actor) const
{
    return AimComponent->ApplyAimCorrectionForTargetActor(this, Actor);
}
