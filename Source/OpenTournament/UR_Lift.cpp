// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Lift.h"

#include "TimerManager.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

#include "OpenTournament.h"
#include "UR_LogChannels.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_Lift::AUR_Lift(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , TravelDuration(1.f)
    , StoppedAtEndPosition(2.f)
    , EndRelativeLocation(FVector(0.f, 0.f, 100.f))
    , EaseIn(true)
    , EaseOut(true)
    , LiftStartSound(nullptr)
    , LiftMovingSound(nullptr)
    , LiftEndSound(nullptr)
{
    BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
    BoxComponent->SetBoxExtent(FVector(50, 50, 30));
    SetRootComponent(BoxComponent);
    BoxComponent->SetGenerateOverlapEvents(true);
    BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnTriggerEnter);
    BoxComponent->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnTriggerExit);

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);

    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);

    EndRelativeLocation = RootComponent->GetComponentLocation() + FVector::UpVector * 100;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Lift::BeginPlay()
{
    Super::BeginPlay();

    StartLocation = RootComponent->GetComponentLocation();
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Lift::OnTriggerEnter(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (bIsTriggered == false && LiftState == ELiftState::Start)
    {
        MoveToEndPosition();
    }

    bIsTriggered = true;
    ActorsOnTrigger.AddUnique(Other);

    GAME_LOG(LogGame, Log, "Entered Lift (%s) Trigger Region", *GetName());
}

void AUR_Lift::OnTriggerExit(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    ActorsOnTrigger.Remove(Other);
    bIsTriggered = ActorsOnTrigger.Num() > 0;

    GAME_LOG(LogGame, Log, "Exited Lift (%s) Trigger Region", *GetName());

    if (bIsTriggered)
    {
        GAME_LOG(LogGame, Log, "Exited Lift (%s) is not Empty", *GetName());
    }
}

void AUR_Lift::MoveToStartPosition()
{
    const FLatentActionInfo LatentActionInfo{ 1, 1, TEXT("OnReachedStart"), this };

    UKismetSystemLibrary::MoveComponentTo(RootComponent, StartLocation, FRotator::ZeroRotator, EaseOut, EaseIn, TravelDuration, true, EMoveComponentAction::Type::Move, LatentActionInfo);

    LiftState = ELiftState::Moving;

    PlayLiftEffects();
}

void AUR_Lift::MoveToEndPosition()
{
    const FLatentActionInfo LatentActionInfo{ 1, 1, TEXT("OnReachedEnd"), this };

    UKismetSystemLibrary::MoveComponentTo(RootComponent, StartLocation + EndRelativeLocation, FRotator::ZeroRotator, EaseOut, EaseIn, TravelDuration, true, EMoveComponentAction::Type::Move, LatentActionInfo);

    LiftState = ELiftState::Moving;
    PlayLiftEffects();
}

void AUR_Lift::OnReachedStart()
{
    LiftState = ELiftState::Start;
    StopLiftEffects();
}

void AUR_Lift::OnReachedEnd()
{
    LiftState = ELiftState::End;
    StopLiftEffects();
    GetWorld()->GetTimerManager().SetTimer(ReturnTimerHandle, this, &AUR_Lift::MoveToStartPosition, StoppedAtEndPosition);
}

void AUR_Lift::PlayLiftEffects_Implementation()
{
    if (LiftStartSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, LiftEndSound, MeshComponent->GetComponentLocation());
    }

    if (LiftMovingSound)
    {
        AudioComponent->SetSound(LiftMovingSound);
        AudioComponent->Play();
    }
}

void AUR_Lift::StopLiftEffects_Implementation()
{
    AudioComponent->Stop();

    if (LiftEndSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), LiftEndSound, MeshComponent->GetComponentLocation());
    }
}
