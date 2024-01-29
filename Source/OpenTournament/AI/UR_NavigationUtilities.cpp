
#include "UR_NavigationUtilities.h"

#include <Navigation/PathFollowingComponent.h>

#include "AIController.h"
#include "NavLinkCustomComponent.h"

// le sigh
class UPathFollowingComponentAccessHack : UPathFollowingComponent
{
public:
    void ForceFinishCurrentSegment()
    {
        OnSegmentFinished();
        SetNextMoveSegment();
    }
};

void UUR_NavigationUtilities::ForceReachedDestinationWithin(APawn* Pawn, const FBoxSphereBounds& Bounds, bool bSphereBounds)
{
    if (Pawn)
    {
        if (auto AIController = Pawn->GetController<AAIController>())
        {
            if (auto PathFollowing = AIController->GetPathFollowingComponent())
            {
                const uint32 TargetIndex = PathFollowing->GetNextPathIndex();
                const FNavigationPath* PathInstance = PathFollowing->GetPath().Get();
                if (PathInstance && PathInstance->GetPathPoints().IsValidIndex(TargetIndex))
                {
                    const FNavPathPoint& TargetPoint = PathInstance->GetPathPoints()[TargetIndex];
                    const FBoxSphereBounds TargetBounds(TargetPoint, FVector(0), 0);
                    const bool bContainsTarget = bSphereBounds ? Bounds.SpheresIntersect(Bounds, TargetBounds) : Bounds.BoxesIntersect(Bounds, TargetBounds);
                    if (bContainsTarget)
                    {
                        ((UPathFollowingComponentAccessHack*)PathFollowing)->ForceFinishCurrentSegment();
                    }
                }
            }
        }
    }
}

bool UUR_NavigationUtilities::IsTraversingLinkLTR(const FVector& Dest, const AActor* SmartLinkContainer, const float ZTolerance)
{
    // SmartLinkComp is private in NavLinkProxy, so we use this..........
    if (auto SmartLinkComp = SmartLinkContainer->FindComponentByClass<UNavLinkCustomComponent>())
    {
        // Destinations and Link points do not match exactly because of god damn Z offsets
        const FVector& RightPoint = SmartLinkComp->GetEndPoint();
        return Dest.X == RightPoint.X && Dest.Y == RightPoint.Y && FMath::Abs(Dest.Z - RightPoint.Z) < ZTolerance;
    }
    return true;
}
