// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_TriggerZone.h"

#include "Components/ShapeComponent.h"
#include "Components/BoxComponent.h"
#include "Logging/MessageLog.h"
#include "Logging/TokenizedMessage.h"
#include "Misc/MapErrors.h"
#include "Misc/UObjectToken.h"
#include "Net/UnrealNetwork.h"

#include "OpenTournament.h"
#include "UR_Character.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#endif

#define LOCTEXT_NAMESPACE "TriggerZone"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_TriggerZone::AUR_TriggerZone(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer),
    ShapeComponent(nullptr),
    TriggerActorClass(AActor::StaticClass()),
    TriggerActors(),
    GameplayTags(),
    bRequiredTagsExact(false),
    RequiredTags(),
    bExcludedTagsExact(true),
    ExcludedTags()
{
    TriggerActors.Reserve(4);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_TriggerZone::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    if (!ShapeComponent)
    {
        ShapeComponent = FindComponentByClass<UShapeComponent>();
    }

    if (ShapeComponent)
    {
        ShapeComponent->OnComponentBeginOverlap.AddUniqueDynamic(this, &AUR_TriggerZone::OnZoneEnter);
        ShapeComponent->OnComponentEndOverlap.AddUniqueDynamic(this, &AUR_TriggerZone::OnZoneExit);
    }
}

void AUR_TriggerZone::CheckForErrors()
{
    FMessageLog MapCheck("MapCheck");

    if (!ShapeComponent)
    {
        FFormatNamedArguments Arguments;
        Arguments.Add(TEXT("ActorName"), FText::FromString(GetPathName()));
        MapCheck.Warning()
            ->AddToken(FUObjectToken::Create(this))
            ->AddToken(FTextToken::Create(FText::Format(LOCTEXT("MapCheck_Message_TriggerZone", "{ActorName} : TriggerZone actor has NULL ShapeComponent property - please add one."), Arguments)));
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_TriggerZone::InternalZoneEnter(AActor* InActor)
{
    if (!InActor->GetClass()->IsChildOf(TriggerActorClass))
    {
        return;
    }

    if (IsTriggerActor(InActor))
    {
        TriggerActors.AddUnique(InActor);
        OnEnter(InActor);
        OnActorEnter.Broadcast(InActor);
    }
}

void AUR_TriggerZone::OnZoneEnter(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    InternalZoneEnter(Other);
}

void AUR_TriggerZone::ZoneEnter(AActor* InActor)
{
    InternalZoneEnter(InActor);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_TriggerZone::InternalZoneExit(AActor* InActor)
{
    if (!InActor->GetClass()->IsChildOf(TriggerActorClass))
    {
        return;
    }

    if (TriggerActors.Contains(InActor))
    {
        TriggerActors.Remove(InActor);
        OnExit(InActor);
        OnActorExit.Broadcast(InActor);
    }
}

void AUR_TriggerZone::OnZoneExit(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    InternalZoneExit(Other);
}

void AUR_TriggerZone::ZoneExit(AActor* InActor)
{
    InternalZoneExit(InActor);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

bool AUR_TriggerZone::IsTriggerActor_Implementation(const AActor* InActor) const
{
    if (auto TagActor = Cast<IGameplayTagAssetInterface>(InActor))
    {
        // Check if the Character has any Required or Excluded GameplayTags
        FGameplayTagContainer TargetTags;
        TagActor->GetOwnedGameplayTags(TargetTags);

        return IsTriggerByGameplayTags(TargetTags);
    }
    
    return true;
}

bool AUR_TriggerZone::IsTriggerByGameplayTags(const FGameplayTagContainer& TargetTags) const
{
    if (RequiredTags.Num() == 0 || (bRequiredTagsExact && TargetTags.HasAnyExact(RequiredTags)) || (!bRequiredTagsExact && TargetTags.HasAny(RequiredTags)))
    {
        return (ExcludedTags.Num() == 0 || (bExcludedTagsExact && !TargetTags.HasAnyExact(ExcludedTags)) || (!bExcludedTagsExact && TargetTags.HasAny(RequiredTags)));
    }
    else
    {
        return false;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
bool AUR_TriggerZone::CanEditChange(const UProperty* InProperty) const
{
    const bool ParentVal = Super::CanEditChange(InProperty);

    // Can we edit bRequiredTagsExact?
    if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AUR_TriggerZone, bRequiredTagsExact))
    {
        return RequiredTags.Num() > 0;
    }

    // Can we edit bExcludedTagsExact?
    if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AUR_TriggerZone, bExcludedTagsExact))
    {
        return ExcludedTags.Num() > 0;
    }

    return ParentVal;
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOpenTournamentTriggerZoneTest, "OpenTournament.Feature.Levels.LevelFeatures.Actor", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FOpenTournamentTriggerZoneTest::RunTest(const FString& Parameters)
{
    // TODO : Automated Tests

    return true;
}

#endif WITH_DEV_AUTOMATION_TESTS
