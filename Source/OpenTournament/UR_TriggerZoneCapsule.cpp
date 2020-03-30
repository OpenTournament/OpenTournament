// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_TriggerZoneCapsule.h"

#include "Components/CapsuleComponent.h"

#include "OpenTournament.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_TriggerZoneCapsule::AUR_TriggerZoneCapsule(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer),
    CapsuleComponent(nullptr)
{
    CapsuleComponent = Cast<UCapsuleComponent>(CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent")));
    CapsuleComponent->SetCapsuleSize(256.f, 128.f);

    ShapeComponent = CapsuleComponent;
    ShapeComponent->SetCollisionProfileName(TEXT("OverlapOnlyPawn")); //UCollisionProfile::OverlapPawnOnly_Name
}

/////////////////////////////////////////////////////////////////////////////////////////////////

//#if WITH_DEV_AUTOMATION_TESTS
//
//IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOpenTournamentTriggerZoneCapsuleTest, "OpenTournament.Feature.Levels.LevelFeatures.Actor", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
//
//bool FOpenTournamentTriggerZoneCapsuleTest::RunTest(const FString& Parameters)
//{
//    // TODO : Automated Tests
//
//    return true;
//}
//
//#endif WITH_DEV_AUTOMATION_TESTS
