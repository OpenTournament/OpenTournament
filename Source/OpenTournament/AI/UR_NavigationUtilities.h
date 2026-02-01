// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "UR_NavigationUtilities.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class APawn;
class AActor;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
*
*/
UCLASS()
class OPENTOURNAMENT_API UUR_NavigationUtilities : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    /**
    * Force navigation agent to seek next destination if his current destination is within certain bounds.
    * Implementers like JumpPads/Teleporters can call this to tell the agent it has effectively reached the link start within.
    *
    * We need this because JP/TPs are sending bots away before they reach link start, so they keep trying to come back to it.
    * The existing CustomNavLink mechanisms only support hooking movement once the link has started, which doesn't help us.
    *
    * Performs all the necessary checks so this can be called blindly.
    */
    UFUNCTION(BlueprintCallable)
    static void ForceReachedDestinationWithin(APawn* Pawn, const FBoxSphereBounds& Bounds, bool bSphereBounds = false);

    /**
    * Callable from ReceiveSmartLinkReached in BP subclasses of NavLinkProxy,
    * Helps figuring out if agent is traversing Left-to-right or Right-to-left.
    */
    UFUNCTION(BlueprintPure, Meta = (DefaultToSelf = "SmartLinkContainer", AdvancedDisplay = 2))
    static bool IsTraversingLinkLTR(const FVector& Dest, const AActor* SmartLinkContainer, const float ZTolerance = 50);

};
