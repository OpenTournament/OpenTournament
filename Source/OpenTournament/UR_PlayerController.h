// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UR_BasePlayerController.h"

#include "Interfaces/UR_TeamInterface.h"

#include "UR_PlayerController.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations


class UInputAction;
class UInputMappingContext;
class UAudioComponent;

class UUR_AbilitySystemComponent;
class AUR_Character;
class AUR_HUD;
class AUR_PlayerState;
class AUR_Pickup;
class UUR_InputDodgeComponent;
class UUR_Widget_BaseMenu;

class UUR_ChatComponent;
enum class EChatChannel : uint8;

struct FInputActionInstance;

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

    UFUNCTION(BlueprintCallable, Category = "Game|PlayerController")
    AUR_PlayerState* GetGamePlayerState() const;

    UFUNCTION(BlueprintCallable, Category = "Game|PlayerController")
    UUR_AbilitySystemComponent* GetGameAbilitySystemComponent() const;

    UFUNCTION(BlueprintCallable, Category = "Game|PlayerController")
    AUR_HUD* GetGameHUD() const;

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

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> DefaultInterfaceMapping;

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

    /////////////////////////////////////////////////////////////////////////////////////////////////

    // Run a cheat command on the server.
    UFUNCTION(Reliable, Server, WithValidation)
    void ServerCheat(const FString& Msg);

    // Run a cheat command on the server for all players.
    UFUNCTION(Reliable, Server, WithValidation)
    void ServerCheatAll(const FString& Msg);

    /////////////////////////////////////////////////////////////////////////////////////////////////

private:

    void OnToggleScoreboardTriggered(const FInputActionInstance& InputActionInstance);
    void OnHoldScoreboardTriggered(const FInputActionInstance& InputActionInstance);
    void OnHoldScoreboardCompleted(const FInputActionInstance& InputActionInstance);
};
