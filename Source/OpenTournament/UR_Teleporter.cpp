// Fill out your copyright notice in the Description page of Project Settings.

#include "UR_Teleporter.h"

#include "Runtime/Engine/Classes/GameFramework/Controller.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"

#include "UR_Character.h"
//#include "Engine.h"

// Sets default values
AUR_Teleporter::AUR_Teleporter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    BaseCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("BaseCapsule"));
    SetRootComponent(BaseCapsule);

    BaseCapsule->SetCapsuleSize(60.f, 50.f, false);
    BaseCapsule->SetRelativeLocation(FVector{ 0.f, 0.f, 100.f });

    BaseMeshComponent = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("BaseMeshComponent"));
    BaseMeshComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

    // @! TODO : This is something to be defined in the Blueprint asset
    //static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("StaticMesh'/Game/StarterContent/Architecture/SM_AssetPlatform.SM_AssetPlatform'"));
    //BaseMeshComponent->SetStaticMesh(DefaultMesh.Object);

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

    AUR_Character* character = Cast<AUR_Character>(Other);
    if (character == nullptr) // not a player entering the teleporter
        return;

    // TODO(Pedro): we should store the "teleporting" state in the MovementComponent of the actor in order to query it here
    bool isTeleporting = (bFromSweep == false);

    if (isTeleporting)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("[TELEPORTER] Teleported to %s."), *GetName()));
        return;
    }

    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("[TELEPORTER] Entered teleporter %s."), *GetName()));

    if (PerformTeleport(character))
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("[TELEPORTER] Teleported to %s."), *GetName()));
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString(TEXT("[TELEPORTER] Destination not set.")));
    }
}

float AUR_Teleporter::GetExitHeading()
{
    return FMath::RadiansToDegrees(FMath::Atan2(ExitVector.Y, ExitVector.X));
}

bool AUR_Teleporter::PerformTeleport(AActor* TargetActor)
{
    if (DestinationTeleporter == nullptr || TargetActor == nullptr)
    {
        return false;
    }

    auto CharacterVelocityVector{ TargetActor->GetVelocity() };
    AController* CharacterController{ nullptr };
    FRotator ControllerRotation{ FRotator::ZeroRotator };

    if (const auto TargetCharacter{ Cast<ACharacter>(TargetActor) })
    {
        CharacterController = TargetCharacter->GetController();
        ControllerRotation = CharacterController->GetControlRotation();
    }

    // Move Actor to destination teleporter
    TargetActor->SetActorLocation(DestinationTeleporter->GetActorLocation());

    // Rotate heading of the actor to face the ExitDirection vector
    if (DestinationTeleporter->ExitRotationType == EExitRotation::Relative)
    {
        auto actorForwardVector = TargetActor->GetActorForwardVector();

        auto actorVelocityVectorNorm = CharacterVelocityVector;
        actorVelocityVectorNorm.Z = 0;
        actorVelocityVectorNorm.Normalize();

        auto velocityHeading = FMath::RadiansToDegrees(actorVelocityVectorNorm.HeadingAngle());
        auto forwardHeading = FMath::RadiansToDegrees(actorForwardVector.HeadingAngle());
        auto finalForwardHeading = forwardHeading + this->GetExitHeading() - DestinationTeleporter->GetExitHeading();

        if (CharacterController)
        {
            CharacterController->SetControlRotation(FRotator{ ControllerRotation.Pitch, finalForwardHeading, ControllerRotation.Roll });
        }
        else
        {
            FRotator DesiredRotation = FRotator{ TargetActor->GetActorRotation().Pitch, DestinationTeleporter->GetExitHeading(), TargetActor->GetActorRotation().Roll };
            TargetActor->SetActorRotation(DesiredRotation);
        }
    }
    else
    {
        if (CharacterController)
        {
            CharacterController->SetControlRotation(FRotator{ ControllerRotation.Pitch, DestinationTeleporter->GetExitHeading(), ControllerRotation.Roll });
        }
        else
        {
            FRotator DesiredRotation = FRotator{ TargetActor->GetActorRotation().Pitch, DestinationTeleporter->GetExitHeading(), TargetActor->GetActorRotation().Roll };
            TargetActor->SetActorRotation(DesiredRotation);
        }
    }

    // Rotate velocity vector relative to the destination teleporter exit heading
    if (!KeepMomentum)
    {
        TargetActor->GetRootComponent()->ComponentVelocity = FVector::ZeroVector;
    }

    return true;
}
