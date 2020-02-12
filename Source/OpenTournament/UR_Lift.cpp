// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Lift.h"

#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

#include "OpenTournament.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_Lift::AUR_Lift(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer),
    TravelDuration(1.f),
    StoppedAtEndPosition(2.f),
    EndRelativeLocation(FVector(0.f, 0.f, 100.f)),
    EaseIn(true),
    EaseOut(true),
    LiftStartSound(nullptr),
    LiftMovingSound(nullptr),
    LiftEndSound(nullptr)
{
    BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
    BoxComponent->SetBoxExtent(FVector(50, 50, 30));
    SetRootComponent(BoxComponent);
    BoxComponent->SetGenerateOverlapEvents(true);
    BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AUR_Lift::OnTriggerEnter);
    BoxComponent->OnComponentEndOverlap.AddDynamic(this, &AUR_Lift::OnTriggerExit);

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);

    EndRelativeLocation = RootComponent->GetComponentLocation() + FVector::UpVector * 100;

    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
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

    GAME_PRINT(1.f, FColor::White, "[Lift] Entered Lift %s", *GetName());
}

void AUR_Lift::OnTriggerExit(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    ActorsOnTrigger.Remove(Other);
    bIsTriggered = ActorsOnTrigger.Num() > 0;

    GAME_PRINT(1.f, FColor::White, "[Lift] Exited Lift %s", *GetName());

    if (bIsTriggered)
    {
        GAME_PRINT(1.f, FColor::White, "[Lift] Lift %s is not Empty", *GetName());
    }
}

void AUR_Lift::MoveToStartPosition()
{
    FLatentActionInfo LatentActionInfo;
    LatentActionInfo.CallbackTarget = this;
    LatentActionInfo.ExecutionFunction = "OnReachedStart";
    LatentActionInfo.UUID = 1;
    LatentActionInfo.Linkage = 1;
    UKismetSystemLibrary::MoveComponentTo(RootComponent, StartLocation, FRotator::ZeroRotator, EaseOut, EaseIn, TravelDuration, true, EMoveComponentAction::Type::Move, LatentActionInfo);

    LiftState = ELiftState::Moving;

    PlayLiftEffects();
}

void AUR_Lift::MoveToEndPosition()
{
    FLatentActionInfo info;
    info.CallbackTarget = this;
    info.ExecutionFunction = "OnReachedEnd";
    info.UUID = 1;
    info.Linkage = 1;
    UKismetSystemLibrary::MoveComponentTo(RootComponent, StartLocation + EndRelativeLocation, FRotator::ZeroRotator, EaseOut, EaseIn, TravelDuration, true, EMoveComponentAction::Type::Move, info);

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
    GetWorldTimerManager().SetTimer(ReturnTimerHandle, this, &AUR_Lift::MoveToStartPosition, StoppedAtEndPosition);
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