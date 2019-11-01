// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_JumpPad.h"

// Sets default values
AUR_JumpPad::AUR_JumpPad(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer),
	Destination(FVector(0, 0, 1000)),
	JumpTime(2)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	CapsuleComponent->SetCapsuleSize(45.f, 90.f, false);
	SetRootComponent(CapsuleComponent);
	CapsuleComponent->SetGenerateOverlapEvents(true);
	CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &AUR_JumpPad::OnTriggerEnter);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(CapsuleComponent);

	ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystemComponent"));
	ParticleSystemComponent->SetupAttachment(RootComponent);

	Destination = GetActorLocation() + FVector::UpVector * 1000;
}

// Called when the game starts or when spawned
void AUR_JumpPad::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AUR_JumpPad::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AUR_JumpPad::OnTriggerEnter(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const auto TargetCharacter{ Cast<ACharacter>(Other) };

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("[JUMPPAD] Entered jump pad %s"), *GetName()));

	if (TargetCharacter)
	{
		TargetCharacter->LaunchCharacter(CalculateJumpVelocity(TargetCharacter), true, true);
	}
	else
	{
	}
}

void AUR_JumpPad::PlayJumpPadEffects_Implementation()
{
	if (JumpPadLaunchSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), JumpPadLaunchSound, this->GetActorLocation());
	}
}

FVector AUR_JumpPad::CalculateJumpVelocity(AActor* Other)
{
	float gravity = GetWorld()->GetGravityZ();
	FVector targetVector = Destination - this->GetActorLocation();

	float sizeXY = targetVector.Size2D() / JumpTime;
	float sizeZ = targetVector.Z / JumpTime - gravity * JumpTime / 2.0f;

	return targetVector.GetSafeNormal2D() * sizeXY + FVector::UpVector * sizeZ;
}