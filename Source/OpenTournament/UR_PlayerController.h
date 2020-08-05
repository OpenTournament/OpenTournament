// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UR_BasePlayerController.h"
#include "Interfaces/UR_TeamInterface.h"
#include "UR_PlayerController.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations

class UAudioComponent;
class UMaterialParameterCollection;

class AUR_Character;
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
class OPENTOURNAMENT_API AUR_PlayerController : public AUR_BasePlayerController
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

    /**
    * Reference to the global-game MaterialParameterCollection.
    */
    UPROPERTY(EditDefaultsOnly)
    UMaterialParameterCollection* MPC_GlobalGame;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Function bound to "MoveForward" AxisMovement Input
    * @param InValue direction scalar
    */
    virtual void MoveForward(const float InValue);

    /**
    * Function bound to "MoveBackward" AxisMovement Input
    * @param InValue movement direction scalar
    */
    virtual void MoveBackward(const float InValue);

    /**
    * Function bound to "MoveRight" AxisMovement Input
    * @param InValue movement direction scalar
    */
    virtual void MoveRight(const float InValue);

    /**
    * Function bound to "MoveLeft" AxisMovement Input
    * @param InValue movement direction scalar
    */
    virtual void MoveLeft(const float InValue);

    /**
    * Function bound to "MoveUp" AxisMovement Input (used when Flying, Swimming)
    * @param InValue movement direction scalar
    */
    virtual void MoveUp(const float InValue);

    /**
    * Function bound to "MoveDown" AxisMovement Input (used when Flying, Swimming)
    * @param InValue movement direction scalar
    */
    virtual void MoveDown(const float InValue);

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

    /**
    * Called via input to turn at a given rate.
    * @param InRate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
    */
    virtual void TurnAtRate(const float InRate);

    /**
    * Called via input to turn look up/down at a given rate.
    * @param InRate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
    */
    virtual void LookUpAtRate(const float InRate);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Function bound to "Jump" ActionMapping Input. Set the Jump flag.
    */
    virtual void Jump();

    /**
    * Function bound to "Crouch" ActionMapping Input. Try to Crouch.
    */
    virtual void Crouch();

    /**
    * Function bound to "UnCrouch" ActionMapping Input. Try to UnCrouch.
    */
    virtual void UnCrouch();

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Override engine's default StartFire.
    */
    virtual void StartFire(uint8 FireModeNum = 0) override;

    /**
    * Function bound to "Fire" ActionMapping Input. Forward to builtin StartFire.
    */
    virtual void PressedFire();

    /**
    * Forward to character like StartFire does.
    */
    virtual void ReleasedFire();

    /**
    * Function bound to "AltFire" ActionMapping Input.
    */
    virtual void PressedAltFire();

    /**
    * Function bound to "AltFire" ActionMapping Input.
    */
    virtual void ReleasedAltFire();

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
    void K2_ClientMessage(const FString& Message) { ClientMessage(Message); }

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

    //~ Begin TeamInterface
    virtual int32 GetTeamIndex_Implementation() override;
    virtual void SetTeamIndex_Implementation(int32 NewTeamIndex) override;
    //~ End TeamInterface

};
