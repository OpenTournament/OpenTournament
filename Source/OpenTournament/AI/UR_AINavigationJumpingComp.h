// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UR_AINavigationJumpingComp.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AAIController;
class ACharacter;
class UCharacterMovementComponent;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * This component handles making the AI character automatically jump when necessary during navigation.
 *
 * The technique we are using here consists in two parts that go together :
 * 
 * 1. Use jump height as step height in the NavMesh settings.
 *    By doing that, the NavMesh generates seamlessly over everything within jump height range, as if being a free-roaming flat surface.
 *    However with that alone, the bot will get stuck because it doesn't know when/how to jump.
 * 
 * 2. Add a bit of tracing code to the AI, to figure out when to jump.
 *    This can be kept efficient as tracing doesn't need to happen every frame, and doesn't need to go very far.

 * This way we don't limit bot navigation to specific NavLink jump/falldown points which are incredibly inefficient.
 *
 * For falldown points that are outside of jump range, we will rely on either automatic generation (see UR_NavLinkGenerator_Falldown),
 * or manual placement of NavLinks.
 *
 * For jumping over holes in the ground, we still have to rely on manual placement of custum Jump links for now.
 */
UCLASS(HideCategories = (Sockets, Tags, ComponentTick, ComponentReplication, Cooking, AssetUserData, Replication, Collision))
class OPENTOURNAMENT_API UUR_AINavigationJumpingComp : public UActorComponent
{
    GENERATED_BODY()

public:
    UUR_AINavigationJumpingComp();

    UPROPERTY(BlueprintReadOnly, Transient)
    AAIController* AIController;
    UPROPERTY(BlueprintReadOnly, Transient)
    ACharacter* MyChar;
    UPROPERTY(BlueprintReadOnly, Transient)
    UCharacterMovementComponent* CharMoveComp;

    UFUNCTION()
    virtual void SetPawn(APawn* NewPawn);

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float JumpCheckInterval;

    // Reduce capsule radius for traces, to avoid unwanted hits on the side when hugging walls
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float CapsuleRadiusMult;

    // Minimum distance between ground and bottom of trace capsule, to avoid immediately hitting angled terrain
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float MinGroundOffsetForTrace;

    // Distance between head and top of trace capsule, to check for free jumping space (although bonking ceilings isn't really problematic)
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float AboveHeadOffset;

    // Stepping-up only works when steps are almost vertical. Not sure what the exact value is.
    // This is dot between step wall normal and Z axis. 0 means vertical, 0.71 means 45°. There should be no further cases.
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float MaxStepupAngleDot;

    // Show forward capsule traces (blue if hit, cyan if not)
    // If hit results in stepup, show yellow line representing stepup
    // If hit results in simply walking uphill, show purple hit normal
    // If hit results in a jump, show pink hit normal
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDebugTraces;

    // Duration of debug traces shown when hits happen (non-hit traces only last for JumpCheckInterval)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DebugHitDuration;

    // Forward trace distance, should try to roughly match distance to peak jump height (depends on JumpZ and Gravity)
    //UPROPERTY(EditAnywhere, BlueprintReadOnly)
    //float TraceDistance;

    UPROPERTY(BlueprintReadOnly, Transient)
    FTimerHandle CheckJumpTimerHandle;

    UFUNCTION()
    virtual void CheckJump();

protected:

    //~ Begin UActorComponent Interface
    virtual void OnRegister() override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    //~ End UActorComponent Interface
};
