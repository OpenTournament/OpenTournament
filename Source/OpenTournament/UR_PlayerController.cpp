// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_PlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/AudioComponent.h"
#include "GameFramework/SpectatorPawn.h"

#include "UR_Character.h"
#include "UR_ChatComponent.h"
#include "UR_FunctionLibrary.h"
#include "UR_GameMode.h"
#include "UR_HUD.h"
#include "UR_LocalPlayer.h"
#include "UR_MessageHistory.h"
#include "UR_PCInputDodgeComponent.h"
#include "UR_PlayerState.h"
#include "UR_Widget_ScoreboardBase.h"

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
        UE_LOG(LogPlayerController, Warning, TEXT("URPlayerController created but no URLocalPlayer available ?! %s"), *GetDebugName(this));
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

    if (UEnhancedInputLocalPlayerSubsystem* InputSystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
    {
        //TODO: Perhaps a better idea to handle input by pawn so each pawn can separately decide what input actions/mappings it needs
        //probably starts being useful once it's possible to switch pawns during the game, i.e. vehicles
        InputSystem->AddMappingContext(DefaultInputMapping, 0);

        //interface functions like scoreboard/chat should be separate from pawn and get their own mapping context
        InputSystem->AddMappingContext(DefaultInterfaceMapping, 0);
    }

    UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
    //Bind our Input Actions here depending on them being assigned on the PC
    if(EnhancedInputComponent)
    {
        EnhancedInputComponent->BindAction(InputActionMove, ETriggerEvent::Triggered, this, &AUR_PlayerController::OnMoveTriggered);
        EnhancedInputComponent->BindAction(InputActionLook, ETriggerEvent::Triggered, this, &AUR_PlayerController::OnLookTriggered);
        EnhancedInputComponent->BindAction(InputActionJump, ETriggerEvent::Triggered, this, &AUR_PlayerController::OnJumpTriggered);
        EnhancedInputComponent->BindAction(InputActionCrouch, ETriggerEvent::Triggered, this, &AUR_PlayerController::OnCrouchTriggered);
        EnhancedInputComponent->BindAction(InputActionCrouch, ETriggerEvent::Completed, this, &AUR_PlayerController::OnCrouchCompleted);
        EnhancedInputComponent->BindAction(InputActionFire, ETriggerEvent::Triggered, this, &AUR_PlayerController::OnFireTriggered);
        EnhancedInputComponent->BindAction(InputActionFireReleased, ETriggerEvent::Triggered, this, &AUR_PlayerController::OnFireReleased);
        EnhancedInputComponent->BindAction(InputActionAltFire, ETriggerEvent::Triggered, this, &AUR_PlayerController::OnAltFireTriggered);
        EnhancedInputComponent->BindAction(InputActionAltFireReleased, ETriggerEvent::Triggered, this, &AUR_PlayerController::OnAltFireReleased);
        EnhancedInputComponent->BindAction(InputActionThirdFire, ETriggerEvent::Triggered, this, &AUR_PlayerController::OnThirdFireTriggered);
        EnhancedInputComponent->BindAction(InputActionThirdFireReleased, ETriggerEvent::Triggered, this, &AUR_PlayerController::OnThirdFireReleased);
        EnhancedInputComponent->BindAction(InputActionToggleScoreboard, ETriggerEvent::Triggered, this, &AUR_PlayerController::OnToggleScoreboardTriggered);
        EnhancedInputComponent->BindAction(InputActionHoldScoreboard, ETriggerEvent::Triggered, this, &AUR_PlayerController::OnHoldScoreboardTriggered);
        EnhancedInputComponent->BindAction(InputActionHoldScoreboard, ETriggerEvent::Completed, this, &AUR_PlayerController::OnHoldScoreboardCompleted);
    }
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


void AUR_PlayerController::StartFire(uint8 FireModeNum)
{
    //NOTE: here we might want to implement different functions according to FireModeNum, when player is spectating etc.
    Super::StartFire(FireModeNum);
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

void AUR_PlayerController::OnMoveTriggered(const FInputActionInstance& InputActionInstance)
{
    const FVector Move = InputActionInstance.GetValue().Get<FVector>();
    if (URCharacter)
    {
        if (InputDodgeComponent)
        {
            InputDodgeComponent->MovementForwardAxis = Move.X;
            InputDodgeComponent->MovementStrafeAxis = Move.Y;
        }

        URCharacter->MoveForward(Move.X);
        URCharacter->MoveRight(Move.Y);
        URCharacter->MoveForward(Move.Z);
    }
    else if (const auto Spectator = GetSpectatorPawn())
    {
        Spectator->MoveForward(Move.X);
        Spectator->MoveRight(Move.Y);
        Spectator->MoveForward(Move.Z);
    }
}

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
