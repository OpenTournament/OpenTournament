// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_TriggerZoneBox.h"

#include "Components/BoxComponent.h"

#include "OpenTournament.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_TriggerZoneBox)

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_TriggerZoneBox::AUR_TriggerZoneBox(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , BoxComponent(nullptr)
{
    BoxComponent = Cast<UBoxComponent>(CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent")));
    BoxComponent->SetBoxExtent(FVector{ 256.f, 256.f, 256.f });

    ShapeComponent = BoxComponent;
    ShapeComponent->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
    SetRootComponent(ShapeComponent);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

//#if WITH_DEV_AUTOMATION_TESTS
//
//IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOpenTournamentTriggerZoneBoxTest, "OpenTournament.Feature.Levels.LevelFeatures.Actor", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
//
//bool FOpenTournamentTriggerZoneBoxTest::RunTest(const FString& Parameters)
//{
//    // TODO : Automated Tests
//
//    return true;
//}
//
//#endif WITH_DEV_AUTOMATION_TESTS
