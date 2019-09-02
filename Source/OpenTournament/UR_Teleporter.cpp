// Fill out your copyright notice in the Description page of Project Settings.

#include "UR_Teleporter.h"

#include "Runtime/Engine/Classes/GameFramework/Controller.h"
#include "Runtime/Engine/Classes/Components/ArrowComponent.h"
#include "Runtime/Engine/Classes/Components/AudioComponent.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystemComponent.h"
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"

#include "UR_Character.h"

// Sets default values
AUR_Teleporter::AUR_Teleporter(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer),
    DestinationActor(nullptr),
    ExitRotationType(EExitRotation::Relative),
    bKeepMomentum(true)
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    BaseMeshComponent = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("BaseMeshComponent"));
    SetRootComponent(BaseCapsule);

    // @! TODO : Attachment positioning is messed up, offset by some values. Resolve this
    ArrowComponent = ObjectInitializer.CreateDefaultSubobject<UArrowComponent>(this, TEXT("ArrowComponent"));
    ArrowComponent->SetupAttachment(RootComponent);
    ArrowComponent->SetRelativeLocation(FVector{ 0.f, 0.f, 45.f });

    AudioComponent = ObjectInitializer.CreateDefaultSubobject<UAudioComponent>(this, TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    ArrowComponent->SetRelativeLocation(FVector{ 0.f, 0.f, 45.f });

    ParticleSystemComponent = ObjectInitializer.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("ParticleSystemComponent"));
    ParticleSystemComponent->SetRelativeLocation(FVector{ 0.f, 0.f, 45.f });

    BaseCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("BaseCapsule"));
    BaseCapsule->SetCapsuleSize(45.f, 90.f, false);
    ParticleSystemComponent->SetupAttachment(RootComponent);
    BaseCapsule->SetRelativeLocation(FVector{ 0.f, 0.f, 90.f });
    BaseCapsule->SetGenerateOverlapEvents(true);
    BaseCapsule->OnComponentBeginOverlap.AddDynamic(this, &AUR_Teleporter::OnTriggerEnter);
}

// Called when the game starts or when spawned
void AUR_Teleporter::BeginPlay()
{
    Super::BeginPlay();

}

// Called every frame
void AUR_Teleporter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

void AUR_Teleporter::OnTriggerEnter(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // @! TODO : Check to see if the component/actor overlapping here matches a LD-specifiable list of classes (e.g. if we want to teleport only characters, or if things such as projectiles, vehicles, etc. may also pass through)
    AUR_Character* Character = Cast<AUR_Character>(Other);
    if (Character == nullptr) // not a player entering the teleporter
    {
        return;
    }

    // TODO(Pedro): we should store the "teleporting" state in the MovementComponent of the actor in order to query it here
    bool isTeleporting = (bFromSweep == false);

    if (isTeleporting)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("[TELEPORTER] Teleported to %s."), *GetName()));
        return;
    }

    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("[TELEPORTER] Entered teleporter %s."), *GetName()));

    if (PerformTeleport(Character))
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("[TELEPORTER] Teleported to %s."), *GetName()));
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString(TEXT("[TELEPORTER] Destination not set.")));
    }
}

bool AUR_Teleporter::PerformTeleport(AActor* TargetActor)
{
    if (DestinationActor == nullptr || TargetActor == nullptr)
    {
        return false;
    }

    AController* CharacterController{ nullptr };
    FRotator TargetActorRotation{ FRotator::ZeroRotator };
    FRotator DestinationRotation{ DestinationActor->GetActorRotation() };
    FRotator DesiredRotation{ DestinationRotation };

    if (const auto TargetCharacter{ Cast<ACharacter>(TargetActor) })
    {
        CharacterController = TargetCharacter->GetController();

        if (CharacterController)
        {
            TargetActorRotation = CharacterController->GetControlRotation();
        }
    }

    // Move Actor to destination teleporter
    TargetActor->SetActorLocation(DestinationActor->GetActorLocation());

    // @! TODO Break out into GetDesiredRotation() function
    // Find out Desired Rotation
    if (ExitRotationType == EExitRotation::Relative)
    {
        DesiredRotation = TargetActorRotation + DestinationRotation;
    }
    else
    {
        DestinationRotation = TargetActorRotation;
    }

    // Rotate the TargetActor to face the ExitDirection vector
    if (CharacterController)
    {
        CharacterController->SetControlRotation(DesiredRotation);
    }
    else
    {
        TargetActor->SetActorRotation(DesiredRotation);
    }

    // Rotate velocity vector relative to the destination teleporter exit heading
    if (!bKeepMomentum)
    {
        TargetActor->GetRootComponent()->ComponentVelocity = FVector::ZeroVector;
    }
    else
    {
        // @! TODO Rotate existing TargetActor velocity around our new Rotation 
        // TargetActor->GetRootComponent()->ComponentVelocity = ...
    }

    return true;
}
