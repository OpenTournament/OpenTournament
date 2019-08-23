// Fill out your copyright notice in the Description page of Project Settings.

#include "UR_Lift.h"
#include "Engine.h"

// Sets default values
AUR_Lift::AUR_Lift(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BaseMeshComponent = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("BaseMeshComponent"));
	RootComponent = BaseMeshComponent;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("StaticMesh'/Game/StarterContent/Shapes/Shape_Plane.Shape_Plane'"));
	BaseMeshComponent->SetStaticMesh(DefaultMesh.Object);

	BaseTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Collider"));
	BaseTrigger->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	BaseTrigger->SetBoxExtent(FVector(50, 50, 10));
	BaseTrigger->SetRelativeLocation(FVector(0, 0, 10));

	BaseTrigger->SetGenerateOverlapEvents(true);
	BaseTrigger->OnComponentBeginOverlap.AddDynamic(this, &AUR_Lift::OnTriggerEnter);
	BaseTrigger->OnComponentEndOverlap.AddDynamic(this, &AUR_Lift::OnTriggerExit);

	EndRelativeLocation = RootComponent->GetComponentLocation() + FVector::UpVector * 100;
}

// Called when the game starts or when spawned
void AUR_Lift::BeginPlay()
{
	Super::BeginPlay();

	startLocation = RootComponent->GetComponentLocation();
}

// Called every frame
void AUR_Lift::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AUR_Lift::OnTriggerEnter(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bIsTriggered == false && liftState == ELiftState::Start)
	{
		MoveToEndPosition();
	}

	bIsTriggered = true;
	actorsOnTrigger.AddUnique(Other);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString(TEXT("Lift entered")));
}

void AUR_Lift::OnTriggerExit(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	actorsOnTrigger.Remove(Other);
	bIsTriggered = actorsOnTrigger.Num() > 0;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString(bIsTriggered ? TEXT("Lift not empty") : TEXT("Lift empty")));
}

void AUR_Lift::MoveToStartPosition()
{
	FLatentActionInfo info;
	info.CallbackTarget = this;
	info.ExecutionFunction = "OnReachedStart";
	info.UUID = 1;
	info.Linkage = 1;
	UKismetSystemLibrary::MoveComponentTo(RootComponent, startLocation, FRotator::ZeroRotator, EaseOut, EaseIn, TravelDuration, true, EMoveComponentAction::Type::Move, info);
	
	liftState = ELiftState::Moving;
}

void AUR_Lift::MoveToEndPosition()
{
	FLatentActionInfo info;
	info.CallbackTarget = this;
	info.ExecutionFunction = "OnReachedEnd";
	info.UUID = 1;
	info.Linkage = 1;
	UKismetSystemLibrary::MoveComponentTo(RootComponent, startLocation + EndRelativeLocation, FRotator::ZeroRotator, EaseOut, EaseIn, TravelDuration, true, EMoveComponentAction::Type::Move, info);
	liftState = ELiftState::Moving;
}

void AUR_Lift::OnReachedStart()
{
	liftState = ELiftState::Start;
}

void AUR_Lift::OnReachedEnd()
{
	liftState = ELiftState::End;
	GetWorldTimerManager().SetTimer(returnTimerHandle, this, &AUR_Lift::MoveToStartPosition, StoppedAtEndPosition);
}
