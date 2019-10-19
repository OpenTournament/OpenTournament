// Copyright 2019 Open Tournament Project, All Rights Reserved.

#include "UR_Weapon.h"
#include "UR_InventoryComponent.h"
#include "Engine.h"
#include "OpenTournament.h"
#include "UR_Character.h"


// Sets default values
AUR_Weapon::AUR_Weapon(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Tbox = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Tbox->SetGenerateOverlapEvents(true);
	Tbox->OnComponentBeginOverlap.AddDynamic(this, &AUR_Weapon::OnTriggerEnter);
	Tbox->OnComponentEndOverlap.AddDynamic(this, &AUR_Weapon::OnTriggerExit);

	RootComponent = Tbox;

	SM_TBox = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Box Mesh"));
	SM_TBox->SetupAttachment(RootComponent);
	
	Mesh1P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("WeaponMesh1P"));
	Mesh1P->SetupAttachment(RootComponent);
	Mesh3P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("WeaponMesh3P"));
	Mesh3P->SetupAttachment(Mesh1P);

	Sound = ObjectInitializer.CreateDefaultSubobject<UAudioComponent>(this, TEXT("Sound"));
	Sound->SetupAttachment(RootComponent);

	ProjectileClass = AUR_Projectile::StaticClass();

	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AUR_Weapon::BeginPlay()
{
    Super::BeginPlay();
	Sound->SetActive(false);
}

// Called every frame
void AUR_Weapon::Tick(float DeltaTime)
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

void AUR_Weapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

bool AUR_Weapon::CanFire() const
{
	return false;
}



void AUR_Weapon::Pickup()
{
	Sound->SetActive(true);
	Sound = UGameplayStatics::SpawnSoundAtLocation(this, Sound->Sound, this->GetActorLocation(), FRotator::ZeroRotator, 1.0f, 1.0f, 0.0f, nullptr, nullptr, true);
	PlayerController->InventoryComponent->Add(this);
	AttachWeaponToPawn();
}


void AUR_Weapon::GetPlayer(AActor* Player)
{
	PlayerController = Cast<AUR_Character>(Player);
}

void AUR_Weapon::OnTriggerEnter(UPrimitiveComponent* HitComp, AActor * Other, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	bItemIsWithinRange = true;
	GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, FString::Printf(TEXT("Press E to Pickup %s"), *WeaponName));
	GetPlayer(Other);
}

void AUR_Weapon::OnTriggerExit(UPrimitiveComponent* HitComp, AActor * Other, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
	bItemIsWithinRange = false;
}

EWeaponState::Type AUR_Weapon::GetCurrentState() const
{
	return EWeaponState::Type();
}

int32 AUR_Weapon::GetCurrentAmmo() const
{
	return int32();
}

int32 AUR_Weapon::GetMaxAmmo() const
{
	return int32();
}

USkeletalMeshComponent * AUR_Weapon::GetWeaponMesh() const
{
	return Mesh1P;
}

AUR_Character * AUR_Weapon::GetPawnOwner() const
{
	return nullptr;
}

void AUR_Weapon::AttachMeshToPawn()
{
	this->SetActorHiddenInGame(false);

	if (PlayerController)
	{
		// Remove and hide both first and third person meshes
		DetachMeshFromPawn();

		// For locally controller players we attach both weapons and let the bOnlyOwnerSee, bOwnerNoSee flags deal with visibility.
		FName AttachPoint = PlayerController->GetWeaponAttachPoint();
		if (PlayerController->IsLocallyControlled() == true)
		{
			USkeletalMeshComponent* PawnMesh1p = PlayerController->GetSpecifcPawnMesh(true);
			USkeletalMeshComponent* PawnMesh3p = PlayerController->GetSpecifcPawnMesh(false);
			Mesh1P->SetHiddenInGame(false);
			Mesh3P->SetHiddenInGame(false);
			Mesh1P->AttachToComponent(PawnMesh1p, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
			Mesh3P->AttachToComponent(PawnMesh3p, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
		}
		else
		{
			USkeletalMeshComponent* UseWeaponMesh = GetWeaponMesh();
			USkeletalMeshComponent* UsePawnMesh = PlayerController->GetPawnMesh();
			UseWeaponMesh->AttachToComponent(UsePawnMesh, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
			UseWeaponMesh->SetHiddenInGame(false);
		}
	}
}

void AUR_Weapon::AttachWeaponToPawn()
{
	this->SetActorHiddenInGame(true);
	Tbox->SetGenerateOverlapEvents(false);

	if (PlayerController)
	{
		// For locally controller players we attach both weapons and let the bOnlyOwnerSee, bOwnerNoSee flags deal with visibility.
		FName AttachPoint = PlayerController->GetWeaponAttachPoint();
		if (PlayerController->IsLocallyControlled() == true)
		{
			USkeletalMeshComponent* PawnMesh = PlayerController->GetSpecifcPawnMesh(true);
			this->AttachToComponent(PawnMesh, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
		}
		else
		{
			USkeletalMeshComponent* UsePawnMesh = PlayerController->GetPawnMesh();
			this->AttachToComponent(UsePawnMesh, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
		}
	}
	this->SetActorHiddenInGame(true);
}

void AUR_Weapon::DetachMeshFromPawn()
{
	Mesh1P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	Mesh1P->SetHiddenInGame(true);

	Mesh3P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	Mesh3P->SetHiddenInGame(true);
}

void AUR_Weapon::OnEquip(AUR_Weapon * LastWeapon)
{
	LastWeapon->DetachMeshFromPawn();
	this->AttachMeshToPawn();
}

void AUR_Weapon::OnUnEquip()
{
	DetachMeshFromPawn();
}

bool AUR_Weapon::IsEquipped() const
{
	return equipped;
}

void AUR_Weapon::setEquipped(bool eq)
{
	equipped = eq;
}

bool AUR_Weapon::IsAttachedToPawn() const
{
	return false;
}
