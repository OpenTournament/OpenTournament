// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_NavLinkGenerator_Falldown.h"

#include <KismetTraceUtils.h>
#include <NavigationSystem.h>
#include <AI/NavigationSystemHelpers.h>
#include <NavMesh/RecastNavMesh.h>

#if WITH_EDITORONLY_DATA
#include <Components/BillboardComponent.h>
#include "UObject/ConstructorHelpers.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_NavLinkGenerator_Falldown)

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_NavLinkGenerator_Falldown::AUR_NavLinkGenerator_Falldown()
{
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

#if WITH_EDITORONLY_DATA
    // SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
    // if (!IsRunningCommandlet() && (SpriteComponent != NULL))
    // {
    //     struct FConstructorStatics
    //     {
    //         ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture;
    //         FName ID_Decals;
    //         FText NAME_Decals;
    //         FConstructorStatics() : SpriteTexture(TEXT("/Engine/EditorResources/AI/S_NavLink")), ID_Decals(TEXT("Navigation")), NAME_Decals(NSLOCTEXT("SpriteCategory", "Navigation", "Navigation")) {}
    //     };
    //     static FConstructorStatics ConstructorStatics;
    //     SpriteComponent->Sprite = ConstructorStatics.SpriteTexture.Get();
    //     SpriteComponent->SetRelativeLocation(FVector(0, 0, 200));
    //     SpriteComponent->SetRelativeScale3D(FVector(2.f));
    //     SpriteComponent->bHiddenInGame = true;
    //     SpriteComponent->SetVisibleFlag(true);
    //     SpriteComponent->SpriteInfo.Category = ConstructorStatics.ID_Decals;
    //     SpriteComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_Decals;
    //     SpriteComponent->SetupAttachment(RootComponent);
    //     SpriteComponent->SetAbsolute(false, false, false);
    //     SpriteComponent->bIsScreenSizeScaled = true;
    // }
#endif // WITH_EDITORONLY_DATA

    SetHidden(true);
    SetActorEnableCollision(false);
    SetCanBeDamaged(false);

    // Default configuration
    MaxFalldownHeight = 1200;
    MinLinkAngleDot = 0.4f;
    DistanceByHeightMult = 15.f;
    DistanceByHeightExp = 0.6f;
    MinDistanceBetweenDestinations = 500;

    bDebugNavContours = false;
    bDebugOutgoingCapsules = false;
    bDebugVerticalTraces = false;
    bDebugDiagonalCapsules = false;
    DebugDuration = 30.f;
    DebugSpecificContour = -1;
}

void AUR_NavLinkGenerator_Falldown::OnConstruction(const FTransform& Transform)
{
    // Links are relative to actor. Don't allow transform for simplicity's sake.
    // We allow modifying transform of the Sprite widget directly instead.
    RootComponent->SetWorldTransform(FTransform::Identity);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_NavLinkGenerator_Falldown::Regenerate()
{
    auto NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys)
    {
        UE_LOG(LogTemp, Warning, TEXT("NavSys not available!"));
        return;
    }

    const auto NavData = NavSys->GetMainNavData();
    if (!NavData)
    {
        UE_LOG(LogTemp, Warning, TEXT("NavData not available!"));
        return;
    }

    const auto NavMesh = Cast<ARecastNavMesh>(NavData);
    if (!NavMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("NavMesh not available!"));
        return;
    }

    // Retrieve some relevant properties
    AgentHeight = NavMesh->AgentHeight;
    AgentMaxStepHeight = NavMesh->GetAgentMaxStepHeight(ENavigationDataResolution::Default);
    AgentRadius = NavMesh->AgentRadius;

    FRecastDebugGeometry Geometry;
    FNavTileRef NavTile = FNavTileRef();
    Geometry.bGatherNavMeshEdges = 0;
    NavMesh->BeginBatchQuery();
    NavMesh->GetDebugGeometryForTile(Geometry, NavTile);
    NavMesh->FinishBatchQuery();

    InternalRebuild(Geometry);

    NumGeneratedLinks = PointLinks.Num();

    for (FNavigationLink& Link : PointLinks)
    {
        Link.InitializeAreaClass(/*bForceRefresh=*/true);
    }

    NavSys->UpdateActorInNavOctree(*this);
}

// Navigation point = potential link destination
struct FNavPoint
{
    const FVector& Loc;
    float TracedMaxZ;    // cached trace max height above this point

    FNavPoint(const FVector& V)
        : Loc(V)
        , TracedMaxZ(0)
    {
    }
};

struct FEdgeSegment
{
    FVector A;
    FVector B;
    FVector Normal;

    FEdgeSegment()
    {
    }

    FEdgeSegment(const FVector& InA, const FVector& InB, const FVector& InNormal)
        : A(InA)
        , B(InB)
        , Normal(InNormal)
    {
    }

    // Assume previous segment such that Prev.B == this.A
    void ComputeNormalFromPrevious(const FEdgeSegment& Prev)
    {
        const float Angle = FMath::RadiansToDegrees(FMath::Atan2(B.Y - A.Y, B.X - A.X) - FMath::Atan2(Prev.B.Y - Prev.A.Y, Prev.B.X - Prev.A.X));
        Normal = Prev.Normal.RotateAngleAxis(Angle, FVector::ZAxisVector);
    }
};

void AUR_NavLinkGenerator_Falldown::InternalRebuild(FRecastDebugGeometry& Geometry)
{
    PointLinks.Empty();

    TArray<FEdgeContour> AllContours;

    while (Geometry.NavMeshEdges.Num() > 0)
    {
        const FVector A = Geometry.NavMeshEdges.Pop(EAllowShrinking::No);
        const FVector B = Geometry.NavMeshEdges.Pop(EAllowShrinking::No);
        const FVector& C = FindTriangleInGeometry(Geometry, A, B);
        const FVector& Normal = ComputeEdgeNormalFromTriangle(A, B, C);
        auto& Contour = AllContours.Emplace_GetRef();
        Contour.Emplace(A, B, Normal);
        GatherContour(Geometry.NavMeshEdges, Contour);
    }
    Geometry.NavMeshEdges.Empty(0);

    // Prep structure with all potential destination points
    TArray<FNavPoint> AllPoints;
    AllPoints.Reserve(Geometry.MeshVerts.Num());
    for (const auto& V : Geometry.MeshVerts)
    {
        //TODO: Prefilter useless points (eg. inside geometry...) ?
        // This might get costly. They will get quickly culled once we start tracing anyways.
        AllPoints.Emplace(V);
    }
    Geometry.MeshVerts.Empty(0);

    // Prep some constants outside the loops

    // Z offset should account for potential diagonal 45° slope while walking outside NavMesh, so Radius*Sqrt(2).
    // But the value here will also impose minimum ceiling height. Need a compromise.
    const float CapsuleZOffset = AgentRadius;

    const FCollisionShape& Capsule = FCollisionShape::MakeCapsule(AgentRadius, AgentHeight / 2.f);
    const FCollisionShape& Sphere = FCollisionShape::MakeSphere(AgentRadius);
    const float OutgoingTraceDist = AgentRadius * 3.f;
    FHitResult Hit;

    // When tracing upwards from a destination point, offset up a bit, because NavMesh vertices are slightly into the ground sometimes
    const FVector DestinationZOffset = FVector(0, 0, AgentRadius + AgentHeight / 2.f);
    const FVector HeightTraceVector(0, 0, MaxFalldownHeight);

    const float MinDistanceBetweenDestinationsSquared = MinDistanceBetweenDestinations * MinDistanceBetweenDestinations;

    NumContours = AllContours.Num();
    if (DebugSpecificContour >= 0)
    {
        if (!AllContours.IsValidIndex(DebugSpecificContour))
            DebugSpecificContour = AllContours.Num() - 1;
        AllContours = { AllContours[DebugSpecificContour] };
    }

    // The work begins !

    for (const auto& Contour : AllContours)
    {
        for (int32 i = 0; i < Contour.Num(); i++)
        {
            const FEdgeSegment& Seg = Contour[i];
            if (bDebugNavContours)
            {
                ::DrawDebugLine(GetWorld(), Seg.A, Seg.B, FColor::Green, false, DebugDuration, SDPG_World, 4.f);
            }

            const FVector& Vertex = Seg.B;
            if (bDebugNavContours)
            {
                ::DrawDebugPoint(GetWorld(), Seg.B, 12.f, FColor::Blue, false, DebugDuration, SDPG_World);
            }

            // Average vertex normal
            //TODO: Provide option to compute each vertex with both normals, with tolerance-based skipping
            const FEdgeSegment& Next = Contour[(i + 1) % Contour.Num()];
            const FVector& VertexNormal = ((Seg.Normal + Next.Normal) / 2).GetSafeNormal2D();
            if (bDebugNavContours)
            {
                ::DrawDebugLine(GetWorld(), Vertex, Vertex + 200 * VertexNormal, FColor::Red, false, DebugDuration, SDPG_World, 4.f);
            }

            // First, test if we can fall of this edge.
            // To do that, we do a capsule trace from above this navmesh point, towards the normal (pointing outside nav mesh).
            const FVector& CapsuleStart = Vertex + FVector(0, 0, CapsuleZOffset + Capsule.GetCapsuleHalfHeight());
            const FVector& CapsuleEnd = CapsuleStart + OutgoingTraceDist * VertexNormal;
            bool bHit = SweepTraceHelper(Hit, CapsuleStart, CapsuleEnd, Capsule, "NavLinkGen_CapsuleHorizontal", bDebugOutgoingCapsules);
            if (bHit)
            {
                continue;
            }

            // We can go out, proceed...
            // Find all relevant potential destination points.

            const FVector& CapsuleBottom = Vertex + FVector(0, 0, CapsuleZOffset);
            const float DestinationMinZ = CapsuleBottom.Z - MaxFalldownHeight;
            const float DestinationMaxZ = CapsuleBottom.Z - AgentMaxStepHeight;

            TArray<FNavPoint*> KeepPoints;
            for (auto& Point : AllPoints)
            {
                // Filter points outside of falling height range
                if (Point.Loc.Z < DestinationMinZ || Point.Loc.Z > DestinationMaxZ)
                {
                    continue;
                }

                // Filter if cached trace height doesn't reach high enough
                if (Point.TracedMaxZ > 0 && (Point.Loc.Z + Point.TracedMaxZ) < CapsuleBottom.Z)
                {
                    continue;
                }

                // Filter in front
                if (VertexNormal.Dot((Point.Loc - Vertex).GetSafeNormal2D()) < MinLinkAngleDot)
                {
                    continue;
                }

                // Filter by distance by height
                const float MaxDist = DistanceByHeightMult * FMath::Pow(CapsuleBottom.Z - Point.Loc.Z, DistanceByHeightExp);
                if (FVector::DistXY(Point.Loc, CapsuleBottom) > MaxDist)
                {
                    continue;
                }

                KeepPoints.Emplace(&Point);
            }

            // Helper to test the viability of a potential destination point
            const auto TestPoint = [&](FNavPoint& Point)
            {
                // Do the upward trace if we haven't yet
                if (Point.TracedMaxZ == 0)
                {
                    const FVector& TraceStart = Point.Loc + DestinationZOffset;
                    bool bHit = SweepTraceHelper(Hit, TraceStart, TraceStart + HeightTraceVector, Sphere, "NavLinkGen_SphereVertical", bDebugVerticalTraces);
                    if (bHit)
                    {
                        Point.TracedMaxZ = FMath::Max(0.1f, Hit.Location.Z - TraceStart.Z);
                    }
                    else
                    {
                        Point.TracedMaxZ = HeightTraceVector.Z;
                    }

                    if ((Point.Loc.Z + Point.TracedMaxZ) < CapsuleBottom.Z)
                    {
                        return false;   // not good
                    }
                }

                // Point seems good, do one last trace from capsule to that Z segment
                // The trace towards segment should be slighly downwards, we need to find a good height on it
                const float DistanceToSegment = FVector::DistXY(CapsuleEnd, Point.Loc);
                // if point is really close we don't need to trace
                if (DistanceToSegment > Capsule.GetCapsuleRadius())
                {
                    const FVector TraceEnd(Point.Loc.X, Point.Loc.Y, CapsuleEnd.Z - 0.5f * DistanceToSegment);  // heuristic Z
                    bool bHit = SweepTraceHelper(Hit, CapsuleEnd, TraceEnd, Capsule, "NavLinkGen_CapsuleDiagonal", bDebugDiagonalCapsules);
                    if (bHit)
                    {
                        return false;
                    }
                }

                // OK
                return true;
            };

            // Find nearest viable point

            // Sort by 2D distance to capsule (furthest first)
            KeepPoints.Sort([&](const FNavPoint& A, const FNavPoint& B)
            {
                return FVector::DistSquaredXY(CapsuleBottom, A.Loc) > FVector::DistSquaredXY(CapsuleBottom, B.Loc);
            });

            // Iterate (reverse, nearest first), culling points until we find a viable one
            FNavPoint* FirstDestination = nullptr;
            while (KeepPoints.Num() > 0)
            {
                FNavPoint* Point = KeepPoints.Pop(EAllowShrinking::No);
                if (TestPoint(*Point))
                {
                    FirstDestination = Point;
                    break;
                }
            }
            if (FirstDestination == nullptr)
            {
                continue;
            }

            // Cull points too close to first destination
            KeepPoints.RemoveAll([&](const FNavPoint* Point)
            {
                return FVector::DistSquared(FirstDestination->Loc, Point->Loc) < MinDistanceBetweenDestinationsSquared;
            });

            // Re-sort remaining points by 3D distance to the first destination (furthest first)
            KeepPoints.Sort([&FirstDestination](const FNavPoint& A, const FNavPoint& B)
            {
                return FVector::DistSquared(FirstDestination->Loc, A.Loc) > FVector::DistXY(FirstDestination->Loc, B.Loc);
            });

            // Iterate remaining points (furthest first)
            FNavPoint* SecondDestination = nullptr;
            for (auto& Point : KeepPoints)
            {
                if (TestPoint(*Point))
                {
                    SecondDestination = Point;
                    break;
                }
            }

            // We done!
            AddFalldownLink(Vertex, FirstDestination->Loc);
            if (SecondDestination != nullptr)
            {
                AddFalldownLink(Vertex, SecondDestination->Loc);
            }
        }
    }
}

// Given a segment (AB) of the NavMesh, find the third point of the triangle
const FVector& AUR_NavLinkGenerator_Falldown::FindTriangleInGeometry(const FRecastDebugGeometry& Geometry, const FVector& A, const FVector& B)
{
    const auto& Vertices = Geometry.MeshVerts;
    for (int32 AreaIdx = 0; AreaIdx < RECAST_MAX_AREAS; ++AreaIdx)
    {
        const auto& Area = Geometry.AreaIndices[AreaIdx];
        for (int32 i = 0; i < Area.Num(); i += 3)
        {
            if ((Vertices[Area[i]] == A && Vertices[Area[i + 1]] == B) || (Vertices[Area[i]] == B && Vertices[Area[i + 1]] == A))
            {
                return Vertices[Area[i + 2]];
            }
            if ((Vertices[Area[i]] == A && Vertices[Area[i + 2]] == B) || (Vertices[Area[i]] == B && Vertices[Area[i + 2]] == A))
            {
                return Vertices[Area[i + 1]];
            }
            if ((Vertices[Area[i + 1]] == A && Vertices[Area[i + 2]] == B) || (Vertices[Area[i + 1]] == B && Vertices[Area[i + 2]] == A))
            {
                return Vertices[Area[i]];
            }
        }
    }
    return FVector::ZeroVector;
}

// Given a triangle (ABC), compute the outgoing normal of segment (AB)
FVector AUR_NavLinkGenerator_Falldown::ComputeEdgeNormalFromTriangle(const FVector& A, const FVector& B, const FVector& C)
{
    const FVector& EdgeDir = (B - A).GetSafeNormal2D();
    FVector EdgeNormal(EdgeDir.Y, -EdgeDir.X, 0);
    if (EdgeNormal.Dot((C - A).GetSafeNormal2D()) > 0)
    {
        EdgeNormal = -EdgeNormal;
    }
    return EdgeNormal;
}

// Recursively gather ordered contour segments from navmesh geometry edges, emptying the edges array in the process.
// Contour must contain one initial point. Only gathers the contour containing that point.
void AUR_NavLinkGenerator_Falldown::GatherContour(TArray<FVector>& Edges, FEdgeContour& Contour)
{
    const FEdgeSegment& Prev = Contour.Last();

    const FVector& Search = Prev.B;
    int32 i = -1;
    for (int32 j = 0; j < Edges.Num(); j++)
    {
        //NOTE: need flexible Z comparison because mesh Z is broken at parts
        if (Edges[j].X == Search.X && Edges[j].Y == Search.Y && FMath::Abs(Edges[j].Z - Search.Z) < AgentMaxStepHeight)
        {
            i = j;
            break;
        }
    }

    if (i != -1)
    {
        FEdgeSegment& Seg = Contour.Emplace_GetRef();
        Seg.A = Edges[i];

        if ((i % 2) == 0)
        {
            Seg.B = Edges[i + 1];
            Edges.RemoveAt(i, 2, EAllowShrinking::No);
        }
        else
        {
            Seg.B = Edges[i - 1];
            Edges.RemoveAt(i - 1, 2, EAllowShrinking::No);
        }

        Seg.ComputeNormalFromPrevious(Prev);

        GatherContour(Edges, Contour);
    }
}

//Helper
bool AUR_NavLinkGenerator_Falldown::SweepTraceHelper(FHitResult& Hit, const FVector& Start, const FVector& End, const FCollisionShape& Shape, const char* Tag, bool bDebug)
{
    FCollisionQueryParams Params(Tag, SCENE_QUERY_STAT_ONLY(NavLinkGen), false);

    bool bHit = GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECC_Pawn, Shape, Params);

#if ENABLE_DRAW_DEBUG
    if (bDebug)
    {
        if (Shape.IsCapsule())
            DrawDebugCapsuleTraceSingle(GetWorld(), Start, End, Shape.GetCapsuleRadius(), Shape.GetCapsuleHalfHeight(), EDrawDebugTrace::ForDuration, bHit, Hit, FColor::Green, FColor::Red, DebugDuration);
        else if (Shape.IsSphere())
            DrawDebugSphereTraceSingle(GetWorld(), Start, End, Shape.GetSphereRadius(), EDrawDebugTrace::ForDuration, bHit, Hit, FColor::Green, FColor::Red, DebugDuration);
    }
#endif

    return bHit;
}

/**
* NOTE:
*
* Navigation links support a MaxFallDownLength property (also named "Right Project Height"),
* which we could leverage to avoid having to trace all the way to specific destination points.
* cf. DefaultNavLinkProcessorImpl
*
* We could simply trace outwards the navmesh to see if we go "out", put Link.Right there in the air,
* and let the navigation Processor figure out if it's possible to fall down underneath.
*
* This approach will however generate many unuseable links, and also makes it less obvious how to
* potentially generate multiple destinations per source.
*
* The Processor only does a simple line trace without shape, so it could result in false positives
* if it goes down a slit not large enough to fit a character.
*/

void AUR_NavLinkGenerator_Falldown::AddFalldownLink(const FVector& Source, const FVector& Dest)
{
    // Links are relative to the actor
    const FVector& Root = RootComponent->GetComponentLocation();
    FNavigationLink& Link = PointLinks.Emplace_GetRef(Source - Root, Dest - Root);

    // Points should already be on the NavMesh so no need to project
    Link.MaxFallDownLength = 0.f;

    // Falling down is unidirectional
    Link.Direction = ENavLinkDirection::LeftToRight;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Nav Interface
/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_NavLinkGenerator_Falldown::PostLoad()
{
    Super::PostLoad();

    // not sure what this is for, copied from ANavLinkProxy
    for (FNavigationLink& Link : PointLinks)
    {
        Link.InitializeAreaClass();
    }
}

void AUR_NavLinkGenerator_Falldown::GetNavigationData(FNavigationRelevantData& Data) const
{
    NavigationHelper::ProcessNavLinkAndAppend(&Data.Modifiers, this, PointLinks);
}

FBox AUR_NavLinkGenerator_Falldown::GetNavigationBounds() const
{
    FBox Box;
    for (const auto& Link : PointLinks)
    {
        Box += Link.Left;
        Box += Link.Right;
    }
    return Box;
}

bool AUR_NavLinkGenerator_Falldown::IsNavigationRelevant() const
{
    return PointLinks.Num() > 0;
}
