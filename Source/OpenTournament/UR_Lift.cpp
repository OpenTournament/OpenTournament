// Copyright 2019-2020 Open Tournament Project, All Rights Reserved.

#include "UR_Lift.h"
#include "Engine.h"

// Sets default values
AUR_Lift::AUR_Lift(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)

{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

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
	if (bIsTriggered == false && liftState == ELiftState::LS_Start)
	{
		MoveToEndPosition();
	}

	bIsTriggered = true;
	actorsOnTrigger.AddUnique(Other);

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("[LIFT] Entered lift %s"), *GetName()));
}

void AUR_Lift::OnTriggerExit(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	actorsOnTrigger.Remove(Other);
	bIsTriggered = actorsOnTrigger.Num() > 0;

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("[LIFT] Exited lift %s"), *GetName()));

	if (bIsTriggered)
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("[LIFT] Lift %s is not empty"), *GetName()));
}

void AUR_Lift::MoveToStartPosition()
{
	FLatentActionInfo info;
	info.CallbackTarget = this;
	info.ExecutionFunction = "OnReachedStart";
	info.UUID = 1;
	info.Linkage = 1;
	UKismetSystemLibrary::MoveComponentTo(RootComponent, startLocation, FRotator::ZeroRotator, EaseOut, EaseIn, TravelDuration, true, EMoveComponentAction::Type::Move, info);

	liftState = ELiftState::LS_Moving;

	PlayLiftEffects();
}

void AUR_Lift::MoveToEndPosition()
{
	FLatentActionInfo info;
	info.CallbackTarget = this;
	info.ExecutionFunction = "OnReachedEnd";
	info.UUID = 1;
	info.Linkage = 1;
	UKismetSystemLibrary::MoveComponentTo(RootComponent, startLocation + EndRelativeLocation, FRotator::ZeroRotator, EaseOut, EaseIn, TravelDuration, true, EMoveComponentAction::Type::Move, info);

	liftState = ELiftState::LS_Moving;
	PlayLiftEffects();
}

void AUR_Lift::OnReachedStart()
{
	liftState = ELiftState::LS_Start;
	StopLiftEffects();
}

void AUR_Lift::OnReachedEnd()
{
	liftState = ELiftState::LS_End;
	StopLiftEffects();
	GetWorldTimerManager().SetTimer(returnTimerHandle, this, &AUR_Lift::MoveToStartPosition, StoppedAtEndPosition);
}

void AUR_Lift::PlayLiftEffects_Implementation()
{
	if (LiftStartSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), LiftEndSound, GetActorLocation());
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
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), LiftEndSound, GetActorLocation());
	}
}