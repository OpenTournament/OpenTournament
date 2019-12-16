// Fill out your copyright notice in the Description page of Project Settings.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Armor.h"
#include "OpenTournament.h"

#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

#include "UR_ArmorComponent.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

// Sets default values
AUR_Armor::AUR_Armor(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer),
    ArmorValue(0.f),
    IsBarrier(false)
{
    CollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionComponent"));
    CollisionComponent->SetGenerateOverlapEvents(true);
    CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AUR_Armor::OnOverlap);

    RootComponent = CollisionComponent;

    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);

    StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
    StaticMeshComponent->SetupAttachment(RootComponent);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_Armor::OnOverlap(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
    GAME_PRINT(2.f, FColor::Red, "Overlapped Armor Pickup (%s)", *this->GetName());

    // @! TODO
    // if (IsPickupAllowed())
    //{
    //    Pickup();
    //}
}

void AUR_Armor::Pickup(UUR_ArmorComponent* ArmorComponent)
{
    UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());

    int32 PriorValue = ArmorComponent->Armor;
    ArmorComponent->SetArmor(PriorValue + ArmorValue);
    ArmorComponent->SetBarrier(IsBarrier);

    //GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("ARMOR VALUE ON INVENTORY: %d"), PlayerController->ArmorComponent->Armor));

    // Need to Handle pickup removal behavior better here
    Destroy();
}
