// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "InputAction.h"
#include "InputMappingContext.h"
#include "UR_BasePlayerController.h"
#include "Interfaces/UR_TeamInterface.h"
#include "UR_PlayerController.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations

class UAudioComponent;

class AUR_Character;
class AUR_PlayerState;
class AUR_Pickup;
class UUR_PCInputDodgeComponent;
class UUR_Widget_BaseMenu;

class UUR_ChatComponent;
enum class EChatChannel : uint8;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
* Event dispatcher for system messages.
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FReceiveSystemMessageSignature, const FString&, Message);

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
UCLASS(Config = Game)
class OPENTOURNAMENT_API AUR_PlayerController
    : public AUR_BasePlayerController
    , public IUR_TeamInterface
{
    GENERATED_BODY()

    AUR_PlayerController(const FObjectInitializer& ObjectInitializer);

    UPROPERTY()
    AUR_Character* URCharacter;

public:
    /////////////////////////////////////////////////////////////////////////////////////////////////

    virtual void BeginPlay() override;

    virtual void SetPlayer(UPlayer* InPlayer) override;

    virtual void InitInputSystem() override;

    virtual void SetupInputComponent() override;

    virtual void ProcessPlayerInput(const float DeltaTime, const bool bGamePaused) override;

    virtual void SetPawn(APawn* InPawn) override;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    UPROPERTY(Config, BlueprintReadWrite, VisibleAnywhere, Meta = (ClampMin = 0.0, ClampMax = 1.0), Category = "Audio")
    float MusicVolumeScalar;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Audio")
    UAudioComponent* MusicComponent;

    UFUNCTION(BlueprintCallable, Category = "PlayerController|Music")
    void PlayMusic(USoundBase* Music, float FadeInDuration);

    UFUNCTION(BlueprintCallable, Category = "PlayerController|Music")
    void StopMusic(float FadeOutDuration);

    UFUNCTION(BlueprintCallable, Category = "PlayerController|Music")
    void SetMusicVolume(float MusicVolume);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Component for handling Dodge Inputs.
    */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PC|Dodge")
    UUR_PCInputDodgeComponent* InputDodgeComponent;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> DefaultInputMapping;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> DefaultInterfaceMapping;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InputActionMove;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InputActionLook;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InputActionJump;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InputActionCrouch;

    //because of the way the fire input stack works, we need additional input actions for the released event
    //Maybe rethink necessity of the input stack design
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InputActionFire;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InputActionFireReleased;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InputActionAltFire;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InputActionAltFireReleased;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InputActionThirdFire;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InputActionThirdFireReleased;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InputActionToggleScoreboard;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InputActionHoldScoreboard;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Base turn rate, in deg/sec. Other scaling may affect final turn rate.
    */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Camera")
    float BaseTurnRate;

    /**
    * Base look up/down rate, in deg/sec. Other scaling may affect final rate.
    */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Camera")
    float BaseLookUpRate;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Override engine's default StartFire.
    */
    virtual void StartFire(uint8 FireModeNum = 0) override;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Component handling Chat Messages
    */
    UPROPERTY(BlueprintReadOnly)
    UUR_ChatComponent* ChatComponent;

    /**
    * Command to send chat message to match channel.
    */
    UFUNCTION(Exec)
    virtual void Say(const FString& Message);

    /**
    * Command to send chat message to team channel.
    * In non-team gamemodes, should fallback to match channel.
    * In spectator, should use spectator channel.
    */
    UFUNCTION(Exec)
    virtual void TeamSay(const FString& Message);

    /**
    * Override ClientMessage to use our systems.
    */
    virtual void ClientMessage_Implementation(const FString& S, FName Type = NAME_None, float MsgLifeTime = 0.f) override;

    /**
    * Event dispatcher for receiving a system message.
    */
    UPROPERTY(BlueprintAssignable)
    FReceiveSystemMessageSignature OnReceiveSystemMessage;

    /**
    * Blueprint hook for ClientMessage.
    */
    UFUNCTION(BlueprintCallable, Exec, Meta = (DisplayName = "Client Message"))
    void K2_ClientMessage(const FString& Message)
    {
        ClientMessage(Message);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////

    //NOTE: might want to move that over to HUD? not sure..
    UPROPERTY(BlueprintReadOnly)
    class UUR_Widget_ScoreboardBase* ScoreboardWidget;

    /**
    * Function bound to "HoldScoreboard" action
    */
    UFUNCTION(Exec, BlueprintCallable, BlueprintCosmetic)
    virtual void ShowScoreboard();

    /**
    * Function bound to "HoldScoreboard" action
    */
    UFUNCTION(Exec, BlueprintCallable, BlueprintCosmetic)
    virtual void HideScoreboard();

    /**
    * Function bound to "ToggleScoreboard" action
    */
    UFUNCTION(Exec, BlueprintCallable, BlueprintCosmetic)
    virtual void ToggleScoreboard()
    {
        ScoreboardWidget ? HideScoreboard() : ShowScoreboard();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////

    UPROPERTY(BlueprintReadWrite)
    bool bWantsThirdPersonCamera;

    UFUNCTION(Exec)
    virtual void BehindView(int32 Switch = -1)
    {
        bWantsThirdPersonCamera = (Switch == -1) ? !bWantsThirdPersonCamera : (bool)Switch;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////

    //~ Begin TeamInterface
    virtual int32 GetTeamIndex_Implementation() override;

    virtual void SetTeamIndex_Implementation(int32 NewTeamIndex) override;

    //~ End TeamInterface

private:

    void OnMoveTriggered(const FInputActionInstance& InputActionInstance);
    void OnLookTriggered(const FInputActionInstance& InputActionInstance);
    void OnJumpTriggered(const FInputActionInstance& InputActionInstance);
    void OnCrouchTriggered(const FInputActionInstance& InputActionInstance);
    void OnCrouchCompleted(const FInputActionInstance& InputActionInstance);
    void OnFireTriggered(const FInputActionInstance& InputActionInstance);
    void OnFireReleased(const FInputActionInstance& InputActionInstance);
    void OnAltFireTriggered(const FInputActionInstance& InputActionInstance);
    void OnAltFireReleased(const FInputActionInstance& InputActionInstance);
    void OnThirdFireTriggered(const FInputActionInstance& InputActionInstance);
    void OnThirdFireReleased(const FInputActionInstance& InputActionInstance);
    void OnToggleScoreboardTriggered(const FInputActionInstance& InputActionInstance);
    void OnHoldScoreboardTriggered(const FInputActionInstance& InputActionInstance);
    void OnHoldScoreboardCompleted(const FInputActionInstance& InputActionInstance);
};
