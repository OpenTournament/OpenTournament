// Copyright (c) 2019 Open Tournament Project, All Rights Reserved.


#include "UR_PickupBase.h"
#include "UnrealNetwork.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UR_Character.h"
#include "GameFramework/LocalMessage.h"
#include "GameFramework/PlayerState.h"
#include "Engine.h"
#include "UR_FunctionLibrary.h"
#include "UR_PlayerController.h"

/**
* NOTES about rotating movement :
*
* Could use RotatingMovementComponent, but I rather cut the overhead by doing directly what we want.
*
* Finished maps can have a lot of pickups in them... weapons, healths, armors, powerups, vials...
* I have seen some UT maps with many rotating pickups, where it had a significant impact on performance.
*
* We should :
* - minimize that impact as much as possible
* - even better, provide a configurable option to disable rotating pickups on client, so we can disable Tick altogether in this class.
*/

AUR_PickupBase::AUR_PickupBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bAllowTickOnDedicatedServer = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	//PrimaryActorTick.bRunOnAnyThread = true;	//CRASH?

	//NOTE: Consider using TickInterval to improve performance.
	// Rotating weapon might not need to update any faster than 60hz,
	// so if player is running at higher FPS some frames could be entirely skipped.

	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	CapsuleComponent->SetCapsuleSize(55.f, 55.f, false);
	CapsuleComponent->SetupAttachment(RootComponent);
	CapsuleComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 55.f));
	CapsuleComponent->SetGenerateOverlapEvents(true);
	/* not sure how to use these in c++
	CapsuleComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CapsuleComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CapsuleComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	*/
	CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &AUR_PickupBase::OnBeginOverlap);

	RotatingComponent = nullptr;
	RotationRate = 180;
	BobbingHeight = 0;
	BobbingSpeed = 1.0f;

	InitialSpawnDelay = 0;
	RespawnTime = 5;
}

void AUR_PickupBase::OnConstruction(const FTransform& Transform)
{
	RespawnTime = FMath::Max(0.0f, RespawnTime);
	PreRespawnEffectDuration = FMath::Clamp(PreRespawnEffectDuration, 0.0f, RespawnTime);
}

void AUR_PickupBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AUR_PickupBase, bRepInitialPickupAvailable, COND_InitialOnly);
}

void AUR_PickupBase::BeginPlay()
{
	Super::BeginPlay();

	if (!IsNetMode(NM_DedicatedServer))
	{
		//TODO: configurable rotating pickups ?
		SetActorTickEnabled(RotatingComponent && (RotationRate != 0.0f || BobbingHeight != 0.0f));

		InitialRelativeLocation = RotatingComponent->GetRelativeTransform().GetLocation();
	}

	if (HasAuthority())
	{
		// Initial availability
		bPickupAvailable = !(InitialSpawnDelay > 0);
		bRepInitialPickupAvailable = bPickupAvailable;

		// Singleplayer
		if (IsNetMode(NM_Standalone))
			ShowPickupAvailable(bPickupAvailable);

		// Initial spawn delay
		if (InitialSpawnDelay > 0.0f)
		{
			float PreSpawnTime = InitialSpawnDelay - PreRespawnEffectDuration;
			if (PreSpawnTime > 0.0f)
			{
				GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, this, &AUR_PickupBase::PreRespawnTimer, PreSpawnTime, false);
			}
			else
			{
				MulticastWillRespawn();
			}
		}
	}
	else
	{
		// Remote initial availability
		// Careful, RepNotify can trigger just before or just after BeginPlay.
		ShowPickupAvailable(bPickupAvailable);
	}
}

void AUR_PickupBase::OnRep_bRepInitialPickupAvailable()
{
	// Remote initial availability
	bPickupAvailable = bRepInitialPickupAvailable;
	ShowPickupAvailable(bPickupAvailable);
}

void AUR_PickupBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!ShouldSkipTick())
	{
		if (RotationRate > 0.0f)
		{
			RotatingComponent->AddLocalRotation(FRotator(0.0f, RotationRate * DeltaTime, 0.0f));
		}
		if (BobbingHeight > 0.0f)
		{
			FVector Loc(InitialRelativeLocation);
			Loc.Z += BobbingHeight * FMath::Sin(BobbingSpeed * PI * GetWorld()->TimeSeconds);
			RotatingComponent->SetRelativeLocation(Loc);
		}
	}
}

// See MovementComponent.cpp @ 329
bool AUR_PickupBase::ShouldSkipTick()
{
	if (!RotatingComponent || !RotatingComponent->IsVisible())
		return true;

	const float RenderTimeThreshold = 0.41f;
	UWorld* TheWorld = GetWorld();

	if (const UPrimitiveComponent* PrimitiveComp = Cast<UPrimitiveComponent>(RotatingComponent))
	{
		if (TheWorld->TimeSince(PrimitiveComp->GetLastRenderTime()) <= RenderTimeThreshold)
			return false; // Rendered, don't skip it.
	}

	// Most components used with movement components don't actually render, so check attached children render times.
	TArray<USceneComponent*> RotatingChildren;
	RotatingComponent->GetChildrenComponents(true, RotatingChildren);
	for (auto Child : RotatingChildren)
	{
		if (const UPrimitiveComponent* PrimitiveChild = Cast<UPrimitiveComponent>(Child))
		{
			if (PrimitiveChild->IsRegistered() && TheWorld->TimeSince(PrimitiveChild->GetLastRenderTime()) <= RenderTimeThreshold)
				return false; // Rendered, don't skip it.
		}
	}

	// No children were recently rendered, safely skip the update.
	return true;
}

void AUR_PickupBase::OnBeginOverlap_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority() ? !bPickupAvailable : !bPickupAvailableLocally)
		return;

	if (AllowPickupBy(OtherActor))
	{
		if (HasAuthority())
			GiveTo(OtherActor);
		else
			SimulateGiveTo(OtherActor);
	}
}

bool AUR_PickupBase::AllowPickupBy_Implementation(AActor* Other)
{
	AUR_Character* Char = Cast<AUR_Character>(Other);
	if (Char && Char->IsAlive())
	{
		if (HasAuthority())
		{
			return true;
		}
		else
		{
			return Char->IsLocallyControlled();
		}
	}
	return false;
}

void AUR_PickupBase::GiveTo_Implementation(AActor* Other)
{
	MulticastPickedUp(Other);
	//NOTE: maybe we should broadcast PickupMessage from here, if we want spectators to see them.
}

#define PICKUP_PREDICTION_CHECK_DELAY 1.5f

void AUR_PickupBase::SimulateGiveTo_Implementation(AActor* LocalClientActor)
{
	PlayPickupEffects();
	ShowPickupAvailable(false);

	// Check that the server actually confirms our pickup.
	if (RespawnTime > PICKUP_PREDICTION_CHECK_DELAY)
	{
		GetWorld()->GetTimerManager().SetTimer(PredictionErrorTimerHandle, this, &AUR_PickupBase::CheckClientPredictionError, PICKUP_PREDICTION_CHECK_DELAY, false);
	}
}

void AUR_PickupBase::CheckClientPredictionError()
{
	if (!bPickupAvailableLocally && bPickupAvailable)
	{
		ShowPickupAvailable(true);
	}
}

void AUR_PickupBase::MulticastPickedUp_Implementation(AActor* Picker)
{
	if (!IsNetMode(NM_DedicatedServer))
	{
		// If we simulated pick up, bPickupAvailableLocally was set to FALSE, and we should not play pick up.

		// Edge case : if player is standing on respawn, server may multicast before client-side RespawnTimer,
		// therefore bPickupAvailableLocally is not TRUE yet.

		// We can use bPickupAvailable in conjunction to check this.
		// Case 1 : we simulate pick up and server confirms => available TRUE / locally FALSE
		// Case 2 : we dont simulate but server says we picked up => available TRUE / locally TRUE
		// Case 3 : stand on respawn, local respawn, simulate pick up, server confirms => available TRUE / locally FALSE (same as case 1)
		// Case 4 : stand on respawn, server says pick up before local respawn => available FALSE / locally FALSE

		// We should only play pick up effects here if we didn't simulate, ie. cases 2 and 4.

		// In short, when this multicast is broadcasted, check goes like this :
		// - bPickupAvailableLocally TRUE means pickup is here locally and we did not simulate picking up.
		// - bPickupAvailable FALSE means pickup hasn't respawned locally just yet.

		if (bPickupAvailableLocally || !bPickupAvailable)
		{
			PlayPickupEffects();
			ShowPickupAvailable(false);

			// avoid RespawnTimer firing just after this (also a case 4 thing)
			GetWorld()->GetTimerManager().ClearTimer(RespawnTimerHandle);
		}

		// We don't simulate the pickup message, that one is only here.
		if (PickupMessage && Picker && UUR_FunctionLibrary::IsLocallyViewed(Picker))
		{
			UUR_FunctionLibrary::GetLocalPlayerController(this)->ClientReceiveLocalizedMessage_Implementation(PickupMessage, 0, nullptr, nullptr, this);
			//NOTE: seems like UE4 LocalMessage framework is *really* barebones.
			// I'm not sure if it's actually relied on, if we're supposed to heavily extend it like UT, or use an alternative.
			// Going to use a print string for now.
		}
		if (Picker && UUR_FunctionLibrary::IsLocallyViewed(Picker))
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, GetPickupText(Picker).ToString());
		}
	}

	bPickupAvailable = false;
	bRepInitialPickupAvailable = false;

	float PreRespawnTime = RespawnTime - PreRespawnEffectDuration;
	if (PreRespawnTime > 0.0f)
	{
		// if we have a respawn time, only trust authority to run timer and multicast at the end
		if (HasAuthority())
		{
			GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, this, &AUR_PickupBase::PreRespawnTimer, PreRespawnTime, false);
		}
	}
	else
	{
		// if instant respawn, client can simulate everything
		MulticastWillRespawn_Implementation();
	}
}

void AUR_PickupBase::ShowPickupAvailable_Implementation(bool bAvailable)
{
	bPickupAvailableLocally = bAvailable;
}

void AUR_PickupBase::PreRespawnTimer()
{
	MulticastWillRespawn();
}

void AUR_PickupBase::MulticastWillRespawn_Implementation()
{
	if (!IsNetMode(NM_DedicatedServer))
	{
		PlayRespawnEffects();
	}

	// If client joins inbetween this and actual respawn, assume it's already spawned.
	bRepInitialPickupAvailable = true;

	// This one timer should be short, client can simulate it.
	if (PreRespawnEffectDuration > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, this, &AUR_PickupBase::RespawnTimer, PreRespawnEffectDuration, false);
	}
	else
	{
		RespawnTimer();
	}
}

void AUR_PickupBase::RespawnTimer()
{
	bPickupAvailable = true;
	bRepInitialPickupAvailable = true;	//just in case, should already be true since WillRespawn

	if (!IsNetMode(NM_DedicatedServer))
	{
		ShowPickupAvailable(true);
	}

	// Check overlaps
	// NOTE: with zero respawn time, avoid infinite pickup loop
	if (RespawnTime > 0.0f)
	{
		TSet<AActor*> Overlaps;
		CapsuleComponent->GetOverlappingActors(Overlaps);
		for (AActor* Other : Overlaps)
		{
			OnBeginOverlap_Implementation(nullptr, Other, nullptr, 0, false, FHitResult());
			if (HasAuthority() ? !bPickupAvailable : !bPickupAvailableLocally)
				break;
		}
	}
}

FText AUR_PickupBase::GetPickupText_Implementation(AActor* Picker)
{
	if (UUR_FunctionLibrary::IsLocallyViewed(Picker))
	{
		//TODO: localize
		return FText::FromString(FString::Printf(TEXT("Picked up %s"), *GetItemName().ToString()));
	}

	FString PickerName(TEXT("Something"));

	APawn* P = Cast<APawn>(Picker);
	if (P && P->GetPlayerState())
	{
		PickerName = P->GetPlayerState()->GetPlayerName();
	}

	return FText::FromString(FString::Printf(TEXT("%s picked up %s"), *PickerName, *GetItemName().ToString()));
}

FText AUR_PickupBase::GetItemName_Implementation()
{
	return FText::FromString("an item");
}
