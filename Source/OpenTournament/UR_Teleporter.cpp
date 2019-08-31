// Fill out your copyright notice in the Description page of Project Settings.

#include "UR_Teleporter.h"

#include "Runtime/Engine/Classes/Engine/Engine.h"
#include "Runtime/Engine/Classes/Components/ArrowComponent.h"
#include "Runtime/Engine/Classes/Components/AudioComponent.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
#include "Runtime/Engine/Classes/GameFramework/Controller.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystemComponent.h"

#include "UR_Character.h"
#include "UR_CharacterMovementComponent.h"

// Sets default values
AUR_Teleporter::AUR_Teleporter(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer),
    DestinationActor(nullptr),
    ExitRotationType(EExitRotation::Relative),
    bKeepMomentum(true)
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;

    CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
    CapsuleComponent->SetCapsuleSize(45.f, 90.f, false);
    SetRootComponent(CapsuleComponent);
    CapsuleComponent->SetGenerateOverlapEvents(true);
    CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &AUR_Teleporter::OnTriggerEnter);

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);

#if WITH_EDITORONLY_DATA
    ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
    ArrowComponent->SetupAttachment(CapsuleComponent);
#endif

    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(CapsuleComponent);

    ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystemComponent"));
    ParticleSystemComponent->SetupAttachment(RootComponent);
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
    FRotator RelativeDestinationRotation{ GetActorRotation() - DestinationRotation };
    FRotator DesiredRotation{ DestinationRotation };

    if (const auto TargetCharacter{ Cast<ACharacter>(TargetActor) })
    {
        CharacterController = TargetCharacter->GetController();

        if (CharacterController)
        {
            TargetActorRotation = CharacterController->GetControlRotation();
        }
    }

    // Play effects associated with teleportation
    PlayTeleportEffects();

    // Move Actor to destination teleporter
    TargetActor->SetActorLocation(DestinationActor->GetActorLocation());

    // Find out Desired Rotation
    GetDesiredRotation(DesiredRotation, TargetActorRotation, RelativeDestinationRotation);

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

        if (AUR_Character* TargetCharacter = Cast<AUR_Character>(TargetActor))
        {
            TargetCharacter->GetMovementComponent()->Velocity = FVector::ZeroVector;
        }
    }
    else
    {
        FVector ForwardVector{ TargetActor->GetActorForwardVector() };

        if (AUR_Character* TargetCharacter = Cast<AUR_Character>(TargetActor))
        {
            if (UCharacterMovementComponent* CharacterMovement = TargetCharacter->GetCharacterMovement())
            {
                FVector MovementAngleVector{ ForwardVector - CharacterMovement->Velocity.Normalize() };
                CharacterMovement->Velocity = CharacterMovement->Velocity.RotateAngleAxis( MovementAngleVector.Rotation().Yaw, DesiredRotation.Vector());
            }
        }
        else
        {
            FVector MovementAngleVector{ ForwardVector - TargetActor->GetVelocity().Normalize() };
            TargetActor->GetRootComponent()->ComponentVelocity = TargetActor->GetVelocity().RotateAngleAxis( MovementAngleVector.Rotation().Yaw, DesiredRotation.Vector());
        }
    }

    return true;
}

void AUR_Teleporter::PlayTeleportEffects_Implementation()
{
    if (TeleportOutSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), TeleportOutSound, GetActorLocation());
    }

    if (TeleportInSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), TeleportInSound, DestinationActor->GetActorLocation());
    }
}

void AUR_Teleporter::GetDesiredRotation(FRotator& DesiredRotation, const FRotator& TargetActorRotation, const FRotator& RelativeDestinationRotation)
{
    if (ExitRotationType == EExitRotation::Relative)
    {
        DesiredRotation = TargetActorRotation + RelativeDestinationRotation;
    }
    else
    {
        DesiredRotation = TargetActorRotation;
    }
}
