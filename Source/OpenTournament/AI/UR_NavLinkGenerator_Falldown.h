// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameFramework/Actor.h"
#include "AI/Navigation/NavRelevantInterface.h"
#include "UR_NavLinkGenerator_Falldown.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UBillboardComponent;
struct FRecastDebugGeometry;

struct FEdgeSegment;
typedef TArray<FEdgeSegment> FEdgeContour;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * TODO
 */
UCLASS(Blueprintable, AutoExpandCategories = (Generator), HideCategories = (Transform, Replication, Collision, HLOD, Physics, Networking, Input, Actor, Cooking))
class OPENTOURNAMENT_API AUR_NavLinkGenerator_Falldown : public AActor, public INavRelevantInterface
{
    GENERATED_BODY()

public:

    AUR_NavLinkGenerator_Falldown();

#if WITH_EDITORONLY_DATA
    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UBillboardComponent> SpriteComponent;
#endif

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Configuration
    /////////////////////////////////////////////////////////////////////////////////////////////////

    UPROPERTY(EditAnywhere, Category = "Generator")
    float MaxFalldownHeight;

    // Minimum angle dot between link and vertex normal
    UPROPERTY(EditAnywhere, Category = "Generator", Meta = (ClampMin = "-1", ClampMax = "1"))
    float MinLinkAngleDot;

    // Max link distance by height by formula MaxDist = Mult.(Height^Exp)
    UPROPERTY(EditAnywhere, Category = "Generator")
    float DistanceByHeightMult;
    UPROPERTY(EditAnywhere, Category = "Generator")
    float DistanceByHeightExp;

    // System will try to find two destinations per source, one nearest and one furthest.
    // Second will be discarded if it is too close to the first.
    UPROPERTY(EditAnywhere, Category = "Generator")
    float MinDistanceBetweenDestinations;

    // Show retrieved NavMesh contours (green) and processed source points (blue) with their outgoing normals (red)
    UPROPERTY(EditAnywhere, Category = "Generator")
    bool bDebugNavContours;

    // Show capsule traces attempting to go outside the NavMesh (along outgoing normals)
    UPROPERTY(EditAnywhere, Category = "Generator")
    bool bDebugOutgoingCapsules;

    // Show sphere traces going up from potential destination points
    UPROPERTY(EditAnywhere, Category = "Generator")
    bool bDebugVerticalTraces;

    // Show capsule traces bridging the two
    UPROPERTY(EditAnywhere, Category = "Generator")
    bool bDebugDiagonalCapsules;

    UPROPERTY(EditAnywhere, Category = "Generator")
    float DebugDuration;

    UPROPERTY(VisibleInstanceOnly, Category = "Generator")
    int32 NumContours;

    UPROPERTY(EditInstanceOnly, Category = "Generator")
    int32 DebugSpecificContour;

    UPROPERTY(VisibleInstanceOnly, Category = "Generator")
    int32 NumGeneratedLinks;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Work
    /////////////////////////////////////////////////////////////////////////////////////////////////

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Generator")
    void Regenerate();

    float AgentHeight;
    float AgentMaxStepHeight;
    float AgentRadius;

    void InternalRebuild(FRecastDebugGeometry& Geometry);

    const FVector& FindTriangleInGeometry(const FRecastDebugGeometry& Geometry, const FVector& A, const FVector& B);
    FVector ComputeEdgeNormalFromTriangle(const FVector& A, const FVector& B, const FVector& C);
    void GatherContour(TArray<FVector>& Edges, FEdgeContour& Contour);

    void AddFalldownLink(const FVector& Source, const FVector& Dest);

    bool SweepTraceHelper(FHitResult& Hit, const FVector& Start, const FVector& End, const FCollisionShape& Shape, const char* Tag, bool bDebug);

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Nav Interface
    /////////////////////////////////////////////////////////////////////////////////////////////////

    UPROPERTY(EditInstanceOnly, Category = "Generated")
    TArray<FNavigationLink> PointLinks;

    virtual void OnConstruction(const FTransform& Transform) override;

    virtual void PostLoad() override;

    // BEGIN INavRelevantInterface
    virtual void GetNavigationData(FNavigationRelevantData& Data) const override;
    virtual FBox GetNavigationBounds() const override;
    virtual bool IsNavigationRelevant() const override;
    // END INavRelevantInterface

};

/////////////////////////////////////////////////////////////////////////////////////////////////
