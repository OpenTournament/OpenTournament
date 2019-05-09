// Copyright 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "UR_PlayerController.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations

class UAudioComponent;
class AUR_Character;
class UUR_PCInputDodgeComponent;

/////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * 
 */
UCLASS(Config = Game)
class OPENTOURNAMENT_API AUR_PlayerController : public APlayerController
{
    GENERATED_BODY()

    AUR_PlayerController();

    UPROPERTY()
    AUR_Character* URCharacter;

public:

    /////////////////////////////////////////////////////////////////////////////////////////////////

    virtual void BeginPlay() override;
    virtual void InitInputSystem() override;
    virtual void SetupInputComponent() override;
    virtual void ProcessPlayerInput(const float DeltaTime, const bool bGamePaused) override;
    virtual void SetPawn(APawn* InPawn) override;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    UPROPERTY(Config, BlueprintReadWrite, VisibleAnywhere, Meta = (ClampMin = 0.0, ClampMax = 1.0), Category = "Audio")
    float MusicVolumeScalar;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Audio")
    UAudioComponent* MusicComponent;

    UFUNCTION(BlueprintCallable, Category = "UnrealRemake|PlayerController|Music")
    void PlayMusic(USoundBase* Music, float FadeInDuration);

    UFUNCTION(BlueprintCallable, Category = "UnrealRemake|PlayerController|Music")
    void StopMusic(float FadeOutDuration);

    UFUNCTION(BlueprintCallable, Category = "UnrealRemake|PlayerController|Music")
    void SetMusicVolume(float MusicVolume);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Component for handling Dodge Inputs.
    */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PC|Dodge")
    UUR_PCInputDodgeComponent* InputDodgeComponent;

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
};
