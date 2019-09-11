// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_Armor.h"
#include "UR_Weapon.h"
#include "UR_ArmorComponent.h"
#include "Engine.h"
#include "OpenTournament.h"
#include "UR_Character.h"

// Sets default values
AUR_Armor::AUR_Armor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	Tbox = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Tbox->SetGenerateOverlapEvents(true);
	Tbox->OnComponentBeginOverlap.AddDynamic(this, &AUR_Armor::OnTriggerEnter);
	Tbox->OnComponentEndOverlap.AddDynamic(this, &AUR_Armor::OnTriggerExit);

	RootComponent = Tbox;

	SM_TBox = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Box Mesh"));
	SM_TBox->SetupAttachment(RootComponent);

	ArmorMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("ArmorMesh"));
	ArmorMesh->SetupAttachment(RootComponent);

	Sound = ObjectInitializer.CreateDefaultSubobject<UAudioComponent>(this, TEXT("Sound"));
	Sound->SetupAttachment(RootComponent);

	PrimaryActorTick.bCanEverTick = true;
}


// Called when the game starts or when spawned
void AUR_Armor::BeginPlay()
{
	Super::BeginPlay();
	Sound->SetActive(false);
}

// Called every frame
void AUR_Armor::Tick(float DeltaTime)
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

void AUR_Armor::Pickup()
{
	Sound->SetActive(true);
	Sound = UGameplayStatics::SpawnSoundAtLocation(this, Sound->Sound, this->GetActorLocation(), FRotator::ZeroRotator, 1.0f, 1.0f, 0.0f, nullptr, nullptr, true);
	int32 value = PlayerController->ArmorComponent->Armor;
	PlayerController->ArmorComponent->SetArmor(value + armorVal);
	PlayerController->ArmorComponent->SetBarrier(armorBarrier);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("ARMOR VALUE ON INVENTORY: %d"), PlayerController->ArmorComponent->Armor));
	Destroy();
}

void AUR_Armor::GetPlayer(AActor * Player)
{
	PlayerController = Cast<AUR_Character>(Player);
}

void AUR_Armor::OnTriggerEnter(UPrimitiveComponent * HitComp, AActor * Other, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	bItemIsWithinRange = true;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("HI this is ARMOR")));
	GetPlayer(Other);
}

void AUR_Armor::OnTriggerExit(UPrimitiveComponent * HitComp, AActor * Other, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("BYE this is ARMOR")));
}


