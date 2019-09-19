// Copyright 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_PlayerController.h"

#include "Engine/World.h"

//UMG
#include "SlateBasics.h"
#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/UMGStyle.h"
#include "Runtime/UMG/Public/Slate/SObjectWidget.h"
#include "Runtime/UMG/Public/IUMGModule.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"

#include "Components/AudioComponent.h"
#include "UR_Character.h"
#include "UR_PCInputDodgeComponent.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_PlayerController::AUR_PlayerController()
{
    MusicVolumeScalar = 1.0;
    MusicComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicComponent"));
    MusicComponent->SetupAttachment(RootComponent);

    BaseTurnRate = 45.f;
    BaseLookUpRate = 45.f;

    // @! TODO Should we add via BP instead ?
    InputDodgeComponent = CreateDefaultSubobject<UUR_PCInputDodgeComponent>(TEXT("InputDodgeComponent"));
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_PlayerController::BeginPlay()
{
    Super::BeginPlay();
}

void AUR_PlayerController::PlayMusic(USoundBase * Music, float FadeInDuration)
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

UUR_PlayerInput* AUR_PlayerController::GetPlayerInput()
{
	return Cast<UUR_PlayerInput>(PlayerInput);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_PlayerController::InitInputSystem()
{
    if (PlayerInput == nullptr)
    {
        PlayerInput = NewObject<UUR_PlayerInput>(this);
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