// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "UR_CharacterMovementComponent.generated.h"


/////////////////////////////////////////////////////////////////////////////////////////////////

UENUM(BlueprintType)
enum class EWallDodgeBehavior : uint8
{
    DisallowSurface,
    RequiresSurface
};

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
* Character Movement Component
*/
UCLASS()
class OPENTOURNAMENT_API UUR_CharacterMovementComponent : public UCharacterMovementComponent
{
    GENERATED_BODY()

public:

    /////////////////////////////////////////////////////////////////////////////////////////////////

    UUR_CharacterMovementComponent(const class FObjectInitializer& ObjectInitializer);


    /////////////////////////////////////////////////////////////////////////////////////////////////
    /// Setup

    /**
    * Setup Movement Properties based on Desired Defaults (e.g. Generation)
    */
    void SetupMovementProperties();

    /**
    * Generation 0 Movement Properties & Behaviors - Using 2.5x Unit Scale
    */
    void SetupMovementPropertiesGeneration0();

    /**
    * Generation 1 Movement Properties & Behaviors - Using 2.5x Unit Scale
    */
    void SetupMovementPropertiesGeneration1();

    /**
    * Generation 2 Movement Properties & Behaviors - Using 2.5x Unit Scale
    */
    void SetupMovementPropertiesGeneration2();

    /**
    * Generation 2 Movement Properties & Behaviors - Using @UNKNOWN Unit Scale
    */
    void SetupMovementPropertiesGeneration2_Scaled();

    /**
    * Generation 3 Movement Properties & Behaviors - Using 2.5x Unit Scale
    */
    void SetupMovementPropertiesGeneration3();

    /**
    * Generation 3 Movement Properties & Behaviors - Using @UNKNOWN Unit Scale
    */
    void SetupMovementPropertiesGeneration3_Scaled();

    /**
    * Generation 4 Movement Properties & Behaviors - Using 2.5x Unit Scale
    */
    void SetupMovementPropertiesGeneration4();

    /**
    * Generation 4 Movement Properties & Behaviors - Using @UNKNOWN Unit Scale
    */
    void SetupMovementPropertiesGeneration4_Scaled();

    /////////////////////////////////////////////////////////////////////////////////////////////////

    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    virtual void ProcessLanded(const FHitResult& Hit, float RemainingTime, int32 Iterations) override;

    /**
    * Override Slope Boosting Behavior
    */
    virtual FVector ComputeSlideVector(const FVector& Delta, const float Time, const FVector& Normal, const FHitResult& Hit) const override;

    /**
    * Handle velocity transformation behavior related to Slope Boosting
    */
    virtual FVector HandleSlopeBoosting(const FVector& SlideResult, const FVector& Delta, const float Time, const FVector& Normal, const FHitResult& Hit) const override;

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

    /**
    * Flag. Are we jumping?
    */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Dodging")
    uint8 bIsJumping:1;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Dodge

    /**
    * Return true if character can dodge
    */
    virtual bool CanDodge() const;

    /**
    * Actually try to calculate & apply our Dodge velocity. True if successful
    */
    bool PerformDodge(FVector& DodgeDir, FVector& DodgeCross);

    /**
     * Modify Velocity (inherited from UCharacterMovementComponent) for Dodge
     */
    void PerformDodgeImpulse(const FVector& DodgeDir, const FVector& DodgeCross);

    /**
     * Modify Velocity (inherited from UCharacterMovementComponent) for WallDodge
     */
    void PerformWallDodgeImpulse(FVector& DodgeDir, FVector& DodgeCross);

    /**
     * Find VelocityZ based on Configurable Movement Parameters.
     * This supports different behaviors such as Z-Override, partial Z-Override, and
     * Z-Inheritance on WallDodges.
     */
    float GetWallDodgeVerticalImpulse() const;

    /**
     * Trace to determine if a WallDodge-permitting surface was hit. Modify HitResult for additional information.
     */
    bool TraceWallDodgeSurface(const FVector& DodgeDir, OUT FHitResult& HitResult) const;

    /**
    * Determine a valid Direction for WallDodge
    */
    void SetWallDodgeDirection(OUT FVector& DodgeDir, OUT FVector& DodgeCross, const FHitResult& HitResult) const;

    /**
    * Clear the dodge input direction flag
    */
    virtual void ClearDodgeInput();

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

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Slope Dodging

    /**
    * Can this character SlopeBoost
    */
    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Dodging|SlopeBoost")
    bool bCanSlopeBoost;

    /**
    * Threshold beyond which we receive SlopeBoost assistance
    */
    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Dodging|SlopeBoost")
    float SlopeBoostAssistVelocityZThreshold;

    /**
    * Scale of Velocity for SlopeBoost assistance
    */
    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Dodging|SlopeBoost")
    float SlopeBoostScale;

    /**
    * This Z for the Hit is considered a slope
    */
    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Dodging|SlopeBoost")
    float SlopeImpactNormalZ;

    /**
    * Impact Point must be greater than this to slide
    */
    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Dodging|SlopeBoost")
    float SlopeSlideRadiusScale;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Wall Dodging 

    /**
    * Can we WallDodge?
    */
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Dodging|WallDodge")
    bool bCanWallDodge;

    /**
    * Do we require a special surface to WallDodge?
    */
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Dodging|WallDodge")
    EWallDodgeBehavior WallDodgeBehavior;

    /**
    * Distance to trace for Wall Dodges
    */
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Dodging|WallDodge")
    float WallDodgeTraceDistance;

    /**
    * Minimum normal of wall dodge from wall
    */
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Dodging|WallDodge")
    float WallDodgeMinimumNormal;

    /**
    * Wall Dodge impulse in XY plane
    */
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Dodging|WallDodge")
    float WallDodgeImpulseHorizontal;

    /**
    * Vertical impulse for first wall dodge.
    */
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Dodging|WallDodge")
    float WallDodgeImpulseVertical;

    /**
    * Time interval after a WallDodge when another may be attempted
    */
    UPROPERTY(Config, BlueprintReadWrite, EditDefaultsOnly, Category = "Dodging")
    float WallDodgeResetInterval;

    /**
    * Whether the player can BoostDodge (retain Z velocity from jump while walldodging)
    */
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Dodging|WallDodge")
    bool bCanBoostDodge;

    /**
    * Speed at which performing a WallDodge will inherit, rather than reset, our current VelocityZ 
    */
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Dodging|WallDodge")
    float WallDodgeVelocityZPreservationThreshold;

    /**
    * Velocity 
    */
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Dodging|WallDodge")
    float WallDodgeFallingVelocityCancellationThreshold;

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
