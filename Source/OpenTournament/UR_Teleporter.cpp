// Fill out your copyright notice in the Description page of Project Settings.

#include "UR_Teleporter.h"
#include "Engine.h"
#include "UR_Character.h"

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
	// TODO(Pedro): we should store the "teleporting" state in the MovementComponent of the actor in order to query it here
	bool isTeleporting = (bFromSweep == false);

	AUR_Character* OtherCharacter = Cast<AUR_Character>(Other);
	if (OtherCharacter == nullptr) // not a player entering the teleporter
		return;

	if(isTeleporting)
	{
		FRotator controllerRotation = OtherCharacter->Controller->GetControlRotation();

		// Rotate yaw of the player to face the EnterRotation angle
		OtherCharacter->Controller->SetControlRotation(FRotator(controllerRotation.Pitch, EnterRotation, controllerRotation.Roll));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("[TELEPORTER] Teleported to %s."), *GetName()));
		return;
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("[TELEPORTER] Entered teleporter %s."), *GetName()));

	if(DestinationTeleporter == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString(TEXT("[TELEPORTER] Destination not set.")));
		return;
	}

	Other->SetActorLocation(DestinationTeleporter->GetActorLocation());
}
