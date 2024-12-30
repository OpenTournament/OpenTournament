// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_PlayerController.h"

#include <EngineUtils.h>
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "UR_AbilitySystemComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/SpectatorPawn.h"

#include "UR_Character.h"
#include "UR_ChatComponent.h"
#include "UR_CheatManager.h"
#include "UR_FunctionLibrary.h"
#include "UR_GameMode.h"
#include "UR_GameplayTags.h"
#include "UR_HUD.h"
#include "UR_InputComponent.h"
#include "UR_LocalPlayer.h"
#include "UR_LogChannels.h"
#include "UR_MessageHistory.h"
#include "UR_PCInputDodgeComponent.h"
#include "UR_PlayerState.h"
#include "UR_Widget_ScoreboardBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_PlayerController)

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace OpenTournament
{
    namespace Input
    {
        static int32 ShouldAlwaysPlayForceFeedback = 0;
        static FAutoConsoleVariableRef CVarShouldAlwaysPlayForceFeedback(TEXT("OTPC.ShouldAlwaysPlayForceFeedback"),
            ShouldAlwaysPlayForceFeedback,
            TEXT("Should force feedback effects be played, even if the last input device was not a gamepad?"));
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_PlayerController::AUR_PlayerController(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    MusicVolumeScalar = 1.0;
    MusicComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicComponent"));
    MusicComponent->SetupAttachment(RootComponent);

    // @! TODO Should we add via BP instead ?
    InputDodgeComponent = CreateDefaultSubobject<UUR_PCInputDodgeComponent>(TEXT("InputDodgeComponent"));

    ChatComponent = CreateDefaultSubobject<UUR_ChatComponent>(TEXT("ChatComponent"));
    ChatComponent->FallbackOwnerName = TEXT("SOMEBODY");
    ChatComponent->AntiSpamDelay = 1.f;

#if UE_WITH_CHEAT_MANAGER
    CheatClass = UUR_CheatManager::StaticClass();
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_PlayerController::BeginPlay()
{
    Super::BeginPlay();
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_PlayerController::PlayMusic(USoundBase* Music, float FadeInDuration)
{
    if (MusicComponent)
    {
        MusicComponent->SetSound(Music);
        MusicComponent->FadeIn(FadeInDuration, MusicVolumeScalar);
    }
}

void AUR_PlayerController::StopMusic(float FadeOutDuration)
{
    if (MusicComponent)
    {
        MusicComponent->FadeOut(FadeOutDuration, 0.f);
    }
}

void AUR_PlayerController::SetMusicVolume(float MusicVolume)
{
    MusicVolumeScalar = MusicVolume;
    if (MusicComponent)
    {
        MusicComponent->AdjustVolume(0.f, MusicVolumeScalar);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_PlayerController::SetPlayer(UPlayer* InPlayer)
{
    Super::SetPlayer(InPlayer);

    if (this != GetClass()->GetDefaultObject())
    {
        return;
    }

    UUR_LocalPlayer* LocalPlayer = Cast<UUR_LocalPlayer>(GetLocalPlayer());
    if (IsValid(LocalPlayer))
    {
        if (IsValid(LocalPlayer->MessageHistory))
        {
            LocalPlayer->MessageHistory->InitWithPlayer(this);
        }
        else
        {
            UE_LOG(LogPlayerController, Warning, TEXT("No MessageHistory for Controller ?! %s"), *GetDebugName(this));
        }
    }
    else
    {
        UE_LOG(LogPlayerController, Warning, TEXT("UR_PlayerController created but no UR_LocalPlayer available ?! %s"), *GetDebugName(this));
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_PlayerController::ProcessPlayerInput(const float DeltaTime, const bool bGamePaused)
{
    if (InputEnabled())
    {
        Super::ProcessPlayerInput(DeltaTime, bGamePaused);

        if (InputDodgeComponent)
        {
            InputDodgeComponent->ProcessPlayerInput(DeltaTime, bGamePaused);
        }
    }
}

void AUR_PlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
    if (UUR_AbilitySystemComponent* ASC = GetGameAbilitySystemComponent())
    {
        ASC->ProcessAbilityInput(DeltaTime, bGamePaused);
    }

    Super::PostProcessInput(DeltaTime, bGamePaused);
}

void AUR_PlayerController::SetPawn(APawn* InPawn)
{
    AController::SetPawn(InPawn);

    URCharacter = Cast<AUR_Character>(InPawn);

    if (InPawn != nullptr)
    {
        if (auto URHUD = Cast<AUR_HUD>(GetHUD()))
        {
            URHUD->OnHUDRestart();
        }
    }

    // Set Spectating Pawn
}

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_PlayerState* AUR_PlayerController::GetGamePlayerState() const
{
    return CastChecked<AUR_PlayerState>(PlayerState, ECastCheckedType::NullAllowed);
}

UUR_AbilitySystemComponent* AUR_PlayerController::GetGameAbilitySystemComponent() const
{
    const AUR_PlayerState* GamePS = GetGamePlayerState();
    return (GamePS ? GamePS->GetGameAbilitySystemComponent() : nullptr);
}

AUR_HUD* AUR_PlayerController::GetGameHUD() const
{
    return CastChecked<AUR_HUD>(GetHUD(), ECastCheckedType::NullAllowed);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_PlayerController::StartFire(uint8 FireModeNum)
{
    //NOTE: here we might want to implement different functions according to FireModeNum, when player is spectating etc.
    Super::StartFire(FireModeNum);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_PlayerController::Say(const FString& Message)
{
    if (ChatComponent)
    {
        ChatComponent->Send(Message, false);
    }
}

void AUR_PlayerController::TeamSay(const FString& Message)
{
    if (ChatComponent)
    {
        ChatComponent->Send(Message, true);
    }
}

void AUR_PlayerController::ClientMessage_Implementation(const FString& S, FName Type, float MsgLifeTime)
{
    if (OnReceiveSystemMessage.IsBound())
    {
        OnReceiveSystemMessage.Broadcast(S);
    }
    else if (Cast<ULocalPlayer>(Player))    //Super crashes if called during shutdown and fails CastChecked
    {
        Super::ClientMessage_Implementation(S, Type, MsgLifeTime);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_PlayerController::ShowScoreboard()
{
    // @! TODO: Don't do this here
    if (!ScoreboardWidget)
    {
        if (auto GameModeCDO = UUR_FunctionLibrary::GetGameModeDefaultObject<AUR_GameMode>(this))
        {
            if (auto ScoreboardClass = GameModeCDO->ScoreboardClass)
            {
                ScoreboardWidget = CreateWidget<UUR_Widget_ScoreboardBase>(this, ScoreboardClass);
                if (ScoreboardWidget)
                {
                    //TODO: ZORDER???
                    ScoreboardWidget->AddToViewport(5);
                }
            }
        }
    }
}

void AUR_PlayerController::HideScoreboard()
{
    // @! TODO: Don't do this here
    if (ScoreboardWidget)
    {
        ScoreboardWidget->RemoveFromParent();
        ScoreboardWidget = nullptr;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

int32 AUR_PlayerController::GetTeamIndex_Implementation()
{
    if (const auto PS = GetPlayerState<AUR_PlayerState>())
    {
        return IUR_TeamInterface::Execute_GetTeamIndex(PS);
    }
    return -1;
}

void AUR_PlayerController::SetTeamIndex_Implementation(int32 NewTeamIndex)
{
    if (auto PS = GetPlayerState<AUR_PlayerState>())
    {
        IUR_TeamInterface::Execute_SetTeamIndex(PS, NewTeamIndex);
    }
}

/*
void AUR_PlayerController::OnMoveTriggered(const FInputActionInstance& InputActionInstance)
{
    const FVector2D Move = InputActionInstance.GetValue().Get<FVector2D>();
    if (URCharacter)
    {
        if (InputDodgeComponent)
        {
            InputDodgeComponent->MovementForwardAxis = Move.X;
            InputDodgeComponent->MovementStrafeAxis = Move.Y;
        }

        URCharacter->MoveForward(Move.X);
        URCharacter->MoveRight(Move.Y);
    }
    else if (const auto Spectator = GetSpectatorPawn())
    {
        Spectator->MoveForward(Move.X);
        Spectator->MoveRight(Move.Y);
    }
}
*/

void AUR_PlayerController::OnLookTriggered(const FInputActionInstance& InputActionInstance)
{
    const FVector2d Move = InputActionInstance.GetValue().Get<FVector2d>();

    AddYawInput(Move.X);
    AddPitchInput(Move.Y);
}

void AUR_PlayerController::OnJumpTriggered(const FInputActionInstance& InputActionInstance)
{
    if (URCharacter && !IsMoveInputIgnored())
    {
        URCharacter->bPressedJump = true;
    }
}

void AUR_PlayerController::OnCrouchTriggered(const FInputActionInstance& InputActionInstance)
{
    if (URCharacter && !IsMoveInputIgnored())
    {
        URCharacter->Crouch(false);
    }
}

void AUR_PlayerController::OnCrouchCompleted(const FInputActionInstance& InputActionInstance)
{
    if (URCharacter && !IsMoveInputIgnored())
    {
        URCharacter->UnCrouch(false);
    }
}

void AUR_PlayerController::OnFireTriggered(const FInputActionInstance& InputActionInstance)
{
    StartFire(0);
}

void AUR_PlayerController::OnFireReleased(const FInputActionInstance& InputActionInstance)
{
    if (URCharacter)
    {
        URCharacter->PawnStopFire(0);
    }
}

void AUR_PlayerController::OnAltFireTriggered(const FInputActionInstance& InputActionInstance)
{
    StartFire(1);
}

void AUR_PlayerController::OnAltFireReleased(const FInputActionInstance& InputActionInstance)
{
    if (URCharacter)
    {
        URCharacter->PawnStopFire(1);
    }
}

void AUR_PlayerController::OnThirdFireTriggered(const FInputActionInstance& InputActionInstance)
{
    StartFire(2);
}

void AUR_PlayerController::OnThirdFireReleased(const FInputActionInstance& InputActionInstance)
{
    if (URCharacter)
    {
        URCharacter->PawnStopFire(2);
    }
}

void AUR_PlayerController::OnToggleScoreboardTriggered(const FInputActionInstance& InputActionInstance)
{
    ScoreboardWidget ? HideScoreboard() : ShowScoreboard();
}

void AUR_PlayerController::OnHoldScoreboardTriggered(const FInputActionInstance& InputActionInstance)
{
    ShowScoreboard();
}

void AUR_PlayerController::OnHoldScoreboardCompleted(const FInputActionInstance& InputActionInstance)
{
    HideScoreboard();
}

/////////////////////////////////////////////////////////////////////////////////////////////////

bool AUR_PlayerController::ServerCheatAll_Validate(const FString& Msg)
{
    return true;
}

void AUR_PlayerController::ServerCheatAll_Implementation(const FString& Msg)
{
#if USING_CHEAT_MANAGER
    if (CheatManager)
    {
        UE_LOG(LogGame, Warning, TEXT("ServerCheatAll: %s"), *Msg);
        for (TActorIterator<AUR_PlayerController> It(GetWorld()); It; ++It)
        {
            if (AUR_PlayerController* GamePC = (*It))
            {
                GamePC->ClientMessage(GamePC->ConsoleCommand(Msg));
            }
        }
    }
#endif // #if USING_CHEAT_MANAGER
}


bool AUR_PlayerController::ServerCheat_Validate(const FString& Msg)
{
    return true;
}

void AUR_PlayerController::ServerCheat_Implementation(const FString& Msg)
{
#if USING_CHEAT_MANAGER
    if (CheatManager)
    {
        UE_LOG(LogGame, Warning, TEXT("ServerCheat: %s"), *Msg);
        ClientMessage(ConsoleCommand(Msg));
    }
#endif // #if USING_CHEAT_MANAGER
}

/////////////////////////////////////////////////////////////////////////////////////////////////
