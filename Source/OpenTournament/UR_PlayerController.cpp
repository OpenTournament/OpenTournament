// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_PlayerController.h"

#include "Components/AudioComponent.h"
#include "GameFramework/SpectatorPawn.h"

#include "UR_Character.h"
#include "UR_ChatComponent.h"
#include "UR_HUD.h"
#include "UR_LocalPlayer.h"
#include "UR_MessageHistory.h"
#include "UR_PCInputDodgeComponent.h"
#include "UR_FunctionLibrary.h"
#include "UR_GameMode.h"
#include "UR_Widget_ScoreboardBase.h"
#include "UR_PlayerState.h"
#include "UR_GameState.h"

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

    BaseTurnRate = 45.f;
    BaseLookUpRate = 45.f;

    // @! TODO Should we add via BP instead ?
    InputDodgeComponent = CreateDefaultSubobject<UUR_PCInputDodgeComponent>(TEXT("InputDodgeComponent"));

    ChatComponent = CreateDefaultSubobject<UUR_ChatComponent>(TEXT("ChatComponent"));
    ChatComponent->FallbackOwnerName = TEXT("SOMEBODY");
    ChatComponent->AntiSpamDelay = 1.f;
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

    UUR_LocalPlayer* LocalPlayer = Cast<UUR_LocalPlayer>(GetLocalPlayer());
    if (LocalPlayer && LocalPlayer->MessageHistory)
    {
        LocalPlayer->MessageHistory->InitWithPlayer(this);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("URPlayerController created but no URLocalPlayer available ?! %s"), *GetDebugName(this));
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_PlayerController::InitInputSystem()
{
    if (PlayerInput == nullptr)
    {
        // PlayerInput = NewObject<UUTPlayerInput>(this, UUTPlayerInput::StaticClass());
    }

    Super::InitInputSystem();
}

void AUR_PlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // Turning with absolute delta (e.g. mouse)
    InputComponent->BindAxis("Turn", this, &APlayerController::AddYawInput);
    InputComponent->BindAxis("LookUp", this, &APlayerController::AddPitchInput);
    // Turning with analog device (e.g. joystick)
    InputComponent->BindAxis("TurnRate", this, &AUR_PlayerController::TurnAtRate);
    InputComponent->BindAxis("LookUpRate", this, &AUR_PlayerController::LookUpAtRate);

    InputComponent->BindAxis("MoveForward", this, &AUR_PlayerController::MoveForward);
    InputComponent->BindAxis("MoveBackward", this, &AUR_PlayerController::MoveBackward);
    InputComponent->BindAxis("MoveLeft", this, &AUR_PlayerController::MoveLeft);
    InputComponent->BindAxis("MoveRight", this, &AUR_PlayerController::MoveRight);
    InputComponent->BindAxis("MoveUp", this, &AUR_PlayerController::MoveUp);
    InputComponent->BindAxis("MoveDown", this, &AUR_PlayerController::MoveDown);
    InputComponent->BindAction("Jump", IE_Pressed, this, &AUR_PlayerController::Jump);

    InputComponent->BindAction("Crouch", IE_Pressed, this, &AUR_PlayerController::Crouch);
    InputComponent->BindAction("Crouch", IE_Released, this, &AUR_PlayerController::UnCrouch);

    // Forward to StartFire() provided by engine, handles things like spectator, request respawn...
    InputComponent->BindAction("Fire", IE_Pressed, this, &AUR_PlayerController::PressedFire);
    //NOTE: we cannot bind 'Pressed' in PC and 'Released' in Character that just doesn't work...
    InputComponent->BindAction("Fire", IE_Released, this, &AUR_PlayerController::ReleasedFire);

    InputComponent->BindAction("AltFire", IE_Pressed, this, &AUR_PlayerController::PressedAltFire);
    InputComponent->BindAction("AltFire", IE_Released, this, &AUR_PlayerController::ReleasedAltFire);

    InputComponent->BindAction("ThirdFire", IE_Pressed, this, &AUR_PlayerController::PressedThirdFire);
    InputComponent->BindAction("ThirdFire", IE_Released, this, &AUR_PlayerController::ReleasedThirdFire);

    InputComponent->BindAction("ToggleScoreboard", IE_Pressed, this, &AUR_PlayerController::ToggleScoreboard);
    InputComponent->BindAction("HoldScoreboard", IE_Pressed, this, &AUR_PlayerController::ShowScoreboard);
    InputComponent->BindAction("HoldScoreboard", IE_Released, this, &AUR_PlayerController::HideScoreboard);
}

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

void AUR_PlayerController::MoveForward(const float InValue)
{
    if (URCharacter != nullptr && InValue != 0.0f)
    {
        if (InputDodgeComponent)
        {
            InputDodgeComponent->MovementForwardAxis = InValue;
        }

        URCharacter->MoveForward(InValue);
    }
    else if (GetSpectatorPawn() != nullptr)
    {
        GetSpectatorPawn()->MoveForward(InValue);
    }
}

void AUR_PlayerController::MoveBackward(const float InValue)
{
    MoveForward(InValue * -1);
}

void AUR_PlayerController::MoveRight(const float InValue)
{
    if (URCharacter != nullptr && InValue != 0.0f)
    {
        if (InputDodgeComponent)
        {
            InputDodgeComponent->MovementStrafeAxis = InValue;
        }

        URCharacter->MoveRight(InValue);
    }
    else if (GetSpectatorPawn() != nullptr)
    {
        GetSpectatorPawn()->MoveRight(InValue);
    }
}

void AUR_PlayerController::MoveLeft(const float InValue)
{
    MoveRight(InValue * -1);
}

void AUR_PlayerController::MoveUp(const float InValue)
{
    if (URCharacter != nullptr && InValue != 0.0f)
    {
        URCharacter->MoveUp(InValue);
    }
    else if (GetSpectatorPawn() != nullptr)
    {
        GetSpectatorPawn()->MoveUp_World(InValue);
    }
}

void AUR_PlayerController::MoveDown(const float InValue)
{
    MoveUp(InValue * -1);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_PlayerController::TurnAtRate(const float InRate)
{
    // calculate delta for this frame from the rate information
    AddYawInput(InRate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AUR_PlayerController::LookUpAtRate(const float InRate)
{
    // calculate delta for this frame from the rate information
    AddPitchInput(InRate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_PlayerController::Jump()
{
    if (URCharacter != nullptr && !IsMoveInputIgnored())
    {
        URCharacter->bPressedJump = true;
    }
}

void AUR_PlayerController::Crouch()
{
    if (!IsMoveInputIgnored())
    {
        if (URCharacter)
        {
            URCharacter->Crouch(false);
        }
    }
}

void AUR_PlayerController::UnCrouch()
{
    if (URCharacter)
    {
        URCharacter->UnCrouch(false);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_PlayerController::StartFire(uint8 FireModeNum)
{
    //NOTE: here we might want to implement different functions according to FireModeNum, when player is spectating etc.
    Super::StartFire(FireModeNum);
}

//TODO: We might want a better approach here, where we bind keys to exec commands like "StartFire 0 | onrelease StopFire 0"
// So we could get away with just two exec methods, instead of having to forward each input like this.
// Dunno what it takes to achieve this in UE4.

void AUR_PlayerController::PressedFire()
{
    StartFire(0);
}

void AUR_PlayerController::ReleasedFire()
{
    if (URCharacter)
    {
        URCharacter->PawnStopFire(0);
    }
}

void AUR_PlayerController::PressedAltFire()
{
    StartFire(1);
}

void AUR_PlayerController::ReleasedAltFire()
{
    if (URCharacter)
    {
        URCharacter->PawnStopFire(1);
    }
}

void AUR_PlayerController::PressedThirdFire()
{
    StartFire(2);
}

void AUR_PlayerController::ReleasedThirdFire()
{
    if (URCharacter)
    {
        URCharacter->PawnStopFire(2);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_PlayerController::Say(const FString& Message)
{
    if (ChatComponent)
        ChatComponent->Send(Message, false);
}

void AUR_PlayerController::TeamSay(const FString& Message)
{
    if (ChatComponent)
        ChatComponent->Send(Message, true);
}

void AUR_PlayerController::ClientMessage_Implementation(const FString& S, FName Type, float MsgLifeTime)
{
    if (OnReceiveSystemMessage.IsBound())
        OnReceiveSystemMessage.Broadcast(S);
    else if (Cast<ULocalPlayer>(Player))    //Super crashes if called during shutdown and fails CastChecked
        Super::ClientMessage_Implementation(S, Type, MsgLifeTime);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_PlayerController::ShowScoreboard()
{
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
