// Copyright 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"

#include <UR_Type_DodgeDirection.h>

#include "UR_CharacterMovementComponent.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
* 
*/
UCLASS()
class OPENTOURNAMENT_API UUR_CharacterMovementComponent : public UCharacterMovementComponent
{
    GENERATED_BODY()

public:

    /////////////////////////////////////////////////////////////////////////////////////////////////

    UUR_CharacterMovementComponent(const class FObjectInitializer& ObjectInitializer);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

    virtual void ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations) override;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /// Utility

    /**
    * Is this a 3D movement mode?
    */
    FORCEINLINE bool Is3DMovementMode() const
    {
        return (MovementMode == MOVE_Flying) || (MovementMode == MOVE_Swimming);
    }

    /**
    * Custom handling for timestamp & delta time updates.
    */
    virtual float UpdateTimeStampAndDeltaTime(float DeltaTime, FNetworkPredictionData_Client_Character* ClientData);

    /**
    * Adjust movement timers if timestamp was reset
    */
    void AdjustMovementTimers(float Adjustment);

    /**
    * Time the server is using for this move, from client's timestamp
    */
    UPROPERTY()
    float CurrentServerMoveTime;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Jump

    /**
    * Return true if character can jump.
    */
    virtual bool CanJump();

    /**
    * Process any pending jumps or dodges
    */
    void CheckJumpInput(float DeltaTime);

    /**
    * Do Jump
    */
    virtual bool DoJump(bool bReplayingMoves) override;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Dodge

    /**
    * Return true if character can dodge
    */
    virtual bool CanDodge();

    /**
    * Actually try to calculate & apply our Dodge velocity. True if successful
    */
    bool PerformDodge(FVector & DodgeDir, FVector& DodgeCross);

    /**
    * Clear the dodge input direction flag
    */
    virtual void ClearDodgeInput();

    /** True during a dodge. */
    /**
    * Flag. Are we dodging?
    */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Dodging")
    uint8 bIsDodging:1;

    /**
    * Timestamp of next time a character may dodge
    */
    UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Dodging")
    float DodgeResetTime;

    /**
    * Time interval after landing from a dodge when another may be attempted
    */
    UPROPERTY(Config, BlueprintReadWrite, EditDefaultsOnly, Category = "Dodging")
    float DodgeResetInterval;

    /**
    * Dodge velocity impulse in XY
    */
    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Dodging")
    float DodgeImpulseHorizontal;

    /**
    * Dodge vertical impulse to apply
    */
    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Dodging")
    float DodgeImpulseVertical;

    /**
    * Landed dodges scale velocity by this amount
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dodging")
    float DodgeLandingSpeedScale;

    /**
    * Flag used to indicate dodge directionality, indicates a pending dodge
    */
    EDodgeDirection DodgeDirection;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Wall Dodge @! TODO implement walldodging

    /**
    * Current number of current walldodges
    */
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Dodging|WallDodge")
    int32 CurrentWallDodgeCount;

    /**
    * Maximum number of walldodges without landing
    */
    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Dodging|WallDodge")
    int32 MaxWallDodges;

    /////////////////////////////////////////////////////////////////////////////////////////////////
};