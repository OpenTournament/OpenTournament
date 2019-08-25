// Fill out your copyright notice in the Description page of Project Settings.

#include "UR_Teleporter.h"
#include "Engine.h"

// Sets default values
AUR_Teleporter::AUR_Teleporter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BaseMeshComponent = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("BaseMeshComponent"));
	RootComponent = BaseMeshComponent;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("StaticMesh'/Game/StarterContent/Architecture/SM_AssetPlatform.SM_AssetPlatform'"));
	BaseMeshComponent->SetStaticMesh(DefaultMesh.Object);

	BaseTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Collider"));
	BaseTrigger->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	BaseTrigger->SetBoxExtent(FVector(60, 60, 100));
	BaseTrigger->SetRelativeLocation(FVector(0, 0, 100));

	BaseTrigger->SetGenerateOverlapEvents(true);
	BaseTrigger->OnComponentBeginOverlap.AddDynamic(this, &AUR_Teleporter::OnTriggerEnter);

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
	AUR_Character* character = Cast<AUR_Character>(Other);
	if (character == nullptr) // not a player entering the teleporter
		return;

	// TODO(Pedro): we should store the "teleporting" state in the MovementComponent of the actor in order to query it here
	bool isTeleporting = (bFromSweep == false);

	if(isTeleporting)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("[TELEPORTER] Teleported to %s."), *GetName()));
		return;
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("[TELEPORTER] Entered teleporter %s."), *GetName()));

	if(PerformTeleport(character)) 
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("[TELEPORTER] Teleported to %s."), *GetName()));
	else
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString(TEXT("[TELEPORTER] Destination not set.")));
}

float AUR_Teleporter::GetExitHeading()
{
	return FMath::RadiansToDegrees(FMath::Atan2(ExitVector.Y, ExitVector.X));
}

bool AUR_Teleporter::PerformTeleport(AUR_Character* character)
{
	if (DestinationTeleporter == nullptr)
		return false;

	auto characterVelocityVector = character->GetVelocity();

	// Move character to destination teleporter
	character->SetActorLocation(DestinationTeleporter->GetActorLocation());

	auto characterMovement = character->GetMovementComponent();
	auto characterController = character->GetController();

	FRotator controllerRotation = characterController->GetControlRotation();

	characterMovement->StopMovementImmediately();

	// Rotate heading of the player to face the ExitDirection vector
	if (DestinationTeleporter->ExitRotationType == EExitRotation::Relative)
	{
		auto actorForwardVector = character->GetActorForwardVector();

		auto actorVelocityVectorNorm = characterVelocityVector;
		actorVelocityVectorNorm.Z = 0;
		actorVelocityVectorNorm.Normalize();

		auto velocityHeading = FMath::RadiansToDegrees(actorVelocityVectorNorm.HeadingAngle());
		auto forwardHeading = FMath::RadiansToDegrees(actorForwardVector.HeadingAngle());
		auto finalForwardHeading = forwardHeading + this->GetExitHeading() - DestinationTeleporter->GetExitHeading();

		characterController->SetControlRotation(FRotator(controllerRotation.Pitch, finalForwardHeading, controllerRotation.Roll));
	}
	else
	{
		characterController->SetControlRotation(FRotator(controllerRotation.Pitch, DestinationTeleporter->GetExitHeading(), controllerRotation.Roll));
	}

	// Rotate velocity vector relative to the destination teleporter exit heading
	if (KeepMomentum)
	{
		auto finalVelocity = characterVelocityVector.RotateAngleAxis(this->GetExitHeading() - DestinationTeleporter->GetExitHeading(), FVector::UpVector);
		character->LaunchCharacter(finalVelocity, false, false);
	}

	return true;
}
