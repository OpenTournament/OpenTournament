// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_ControlPoint.h"

#include "Components/ChildActorComponent.h"
#include "Kismet/GameplayStatics.h"

#include "OpenTournament.h"
#include "UR_Character.h"
#include "UR_TriggerZone.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_ControlPoint::AUR_ControlPoint(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer),
    bRequiredTagsExact(false),
    bExcludedTagsExact(true)
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;

    TriggerZoneComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("TriggerZoneComponent"));
    if (TriggerZoneClass)
    {
        TriggerZoneComponent->SetChildActorClass(TriggerZoneClass);
        TriggerZoneComponent->CreateChildActor();
    }
    SetRootComponent(TriggerZoneComponent);

    TriggerZone = Cast<AUR_TriggerZone>(TriggerZoneComponent->GetChildActor());
    if (TriggerZone)
    {
        TriggerZone->TriggerActorClass = TriggerZoneActorClass;
        TriggerZone->OnActorEnter.AddDynamic(this, &AUR_ControlPoint::ActorEnter);
        TriggerZone->OnActorExit.AddDynamic(this, &AUR_ControlPoint::ActorExit);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_ControlPoint::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    // Ensure that TriggerZone pointer is set with our ChildActor
    TriggerZone = Cast<AUR_TriggerZone>(TriggerZoneComponent->GetChildActor());

    if (TriggerZone)
    {
        TriggerZone->OnActorEnter.AddUniqueDynamic(this, &AUR_ControlPoint::ActorEnter);
        TriggerZone->OnActorExit.AddUniqueDynamic(this, &AUR_ControlPoint::OnActorExit);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_ControlPoint::ActorEnter(AActor* InActor)
{
    if (IsPermittedToControl(InActor))
    {
        GAME_LOG(Game, Log, "ControlPoint (%s) Actor (%s) permitted", *GetName(), *InActor->GetName());

        OnActorEnter(InActor);
    }
}

void AUR_ControlPoint::OnActorEnter_Implementation(AActor* InActor)
{
    SetPointContestedState(ShouldPointBeContested(EControlPointEvent::Entering, InActor));

    if (!IsPointContested())
    {
        SetPointControlState(true, InActor);
    }
}

void AUR_ControlPoint::ActorExit(AActor* InActor)
{
    if (IsActorInZone(InActor))
    {
        GAME_LOG(Game, Log, "ControlPoint (%s) Actor (%s) Exited", *GetName(), *InActor->GetName());

        OnActorExit(InActor);
    }
}

void AUR_ControlPoint::OnActorExit_Implementation(AActor* InActor)
{
    SetPointContestedState(ShouldPointBeContested(EControlPointEvent::Exiting, InActor));
}

/////////////////////////////////////////////////////////////////////////////////////////////////

bool AUR_ControlPoint::ShouldPointBeContested_Implementation(EControlPointEvent EventType, const AActor* TargetActor) const
{
    // Default implementation does not allow for Contested state
    return false;
}

void AUR_ControlPoint::SetPointContestedState(bool bShouldBeContested, AActor* InActor)
{
    FGameplayTagContainer PointTags;
    GetOwnedGameplayTags(PointTags);

    if (bShouldBeContested)
    {
        PointTags.AddTag(ContestedTag);
        OnPointContested(InActor);
    }
    else
    {
        PointTags.RemoveTag(ContestedTag);
        OnPointUncontested(InActor);
    }
}

bool AUR_ControlPoint::IsPointContested() const
{
    FGameplayTagContainer PointTags;
    GetOwnedGameplayTags(PointTags);

    return PointTags.HasTagExact(ContestedTag);
}

void AUR_ControlPoint::OnPointContested_Implementation(AActor* TargetActor)
{
    // Noop currently. Stub for mesh, audio, and SFX
}

void AUR_ControlPoint::OnPointUncontested_Implementation(AActor* TargetActor)
{    
    // Noop currently. Stub for mesh, audio, and SFX
}

/////////////////////////////////////////////////////////////////////////////////////////////////

bool AUR_ControlPoint::IsPermittedToControl_Implementation(const AActor* TargetActor) const
{
    if (!TargetActor->GetClass()->IsChildOf(TriggerZoneActorClass))
    {
        GAME_LOG(Game, Log, "ControlPoint Error. ActorClass was invalid.");
        return false;
    }

    // Check if the actor being teleported has any Required or Excluded GameplayTags
    FGameplayTagContainer TargetTags;

    if (auto TagActor = Cast<IGameplayTagAssetInterface>(TargetActor))
    {
        TagActor->GetOwnedGameplayTags(TargetTags);
    }

    return IsPermittedByGameplayTags(TargetTags);
}

void AUR_ControlPoint::SetPointControlState(bool bShouldBeControlled, AActor* InActor)
{
    FGameplayTagContainer PointTags;
    GetOwnedGameplayTags(PointTags);

    if (bShouldBeControlled)
    {
        ActorControlTag = GetActorControlTag(InActor);
        PointTags.AddTag(ActorControlTag);
        PointTags.AddTag(ControlTag);

        OnPointControlled(InActor);
    }
    else
    {
        PointTags.RemoveTag(ActorControlTag);
        PointTags.RemoveTag(ControlTag);

        OnPointUncontrolled(InActor);
    }
}

void AUR_ControlPoint::OnPointControlled_Implementation(AActor* InActor)
{
    // Noop currently. Stub for mesh, audio, and SFX changes
}

void AUR_ControlPoint::OnPointUncontrolled_Implementation(AActor* InActor)
{
    // Noop currently. Stub for mesh, audio, and SFX changes
}

/////////////////////////////////////////////////////////////////////////////////////////////////

bool AUR_ControlPoint::IsZoneOccupied() const
{
    bool IsOccupied{ false };

    if (TriggerZone)
    {
        IsOccupied = TriggerZone->TriggerActors.Num() > 0;
    }

    return IsOccupied;
}

bool AUR_ControlPoint::IsActorInZone(AActor* InActor) const
{
    bool IsInZone{ false };

    if (TriggerZone)
    {
        IsInZone = TriggerZone->TriggerActors.Find(InActor) != INDEX_NONE;
    }

    return IsInZone;
}

bool AUR_ControlPoint::IsPermittedByGameplayTags(const FGameplayTagContainer& TargetTags) const
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

FGameplayTag AUR_ControlPoint::GetActorControlTag(const AActor* InActor) const
{
    FGameplayTag OutTag;

    if (auto TagActor = Cast<IGameplayTagAssetInterface>(InActor))
    {
        FGameplayTagContainer ActorTags;
        GetOwnedGameplayTags(ActorTags);

        ActorTags.HasTag(ActorControlTagBase);
        OutTag = ActorTags.GetByIndex(0);
    }

    return OutTag;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
bool AUR_ControlPoint::CanEditChange(const FProperty* InProperty) const
{
    const bool ParentVal = Super::CanEditChange(InProperty);

    // Can we edit bRequiredTagsExact?
    if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AUR_ControlPoint, bRequiredTagsExact))
    {
        return RequiredTags.Num() > 0;
    }

    // Can we edit bExcludedTagsExact?
    if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AUR_ControlPoint, bExcludedTagsExact))
    {
        return ExcludedTags.Num() > 0;
    }

    return ParentVal;
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOpenTournamentControlPointTest, "OpenTournament.Feature.Levels.LevelFeatures.Actor", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FOpenTournamentControlPointTest::RunTest(const FString& Parameters)
{
    // TODO : Automated Tests

    return true;
}

#endif WITH_DEV_AUTOMATION_TESTS
