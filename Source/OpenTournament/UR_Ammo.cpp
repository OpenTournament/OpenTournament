// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Ammo.h"
#include "UR_Weapon.h"
#include "UR_InventoryComponent.h"
#include "Engine.h"
#include "OpenTournament.h"
#include "UR_Character.h"

// Sets default values
AUR_Ammo::AUR_Ammo(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Tbox = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Tbox->SetGenerateOverlapEvents(true);
	Tbox->OnComponentBeginOverlap.AddDynamic(this, &AUR_Ammo::OnTriggerEnter);
	Tbox->OnComponentEndOverlap.AddDynamic(this, &AUR_Ammo::OnTriggerExit);

	RootComponent = Tbox;

	SM_TBox = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Box Mesh"));
	SM_TBox->SetupAttachment(RootComponent);

	AmmoMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("AmmoMesh1"));
	AmmoMesh->SetupAttachment(RootComponent);

	Sound = ObjectInitializer.CreateDefaultSubobject<UAudioComponent>(this, TEXT("Sound"));
	Sound->SetupAttachment(RootComponent);

	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AUR_Ammo::BeginPlay()
{
	Super::BeginPlay();
	Sound->SetActive(false);
}

// Called every frame
void AUR_Ammo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PlayerController != NULL)
	{
		if (bItemIsWithinRange)
		{
			Pickup();
		}
	}
}

void AUR_Ammo::Pickup()
{
	Sound->SetActive(true);
	Sound = UGameplayStatics::SpawnSoundAtLocation(this, Sound->Sound, this->GetActorLocation(), FRotator::ZeroRotator, 1.0f, 1.0f, 0.0f, nullptr, nullptr, true);
	PlayerController->InventoryComponent->Add(this);
	Destroy();
}

void AUR_Ammo::GetPlayer(AActor* Player)
{
	PlayerController = Cast<AUR_Character>(Player);
}

void AUR_Ammo::OnTriggerEnter(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	bItemIsWithinRange = true;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("HI this is ammo")));
	GetPlayer(Other);
}

void AUR_Ammo::OnTriggerExit(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("BYE this is ammo")));
}

