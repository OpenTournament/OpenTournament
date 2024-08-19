// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_EquipmentInstance.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

#include "UR_EquipmentDefinition.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_EquipmentInstance)

/////////////////////////////////////////////////////////////////////////////////////////////////

class FLifetimeProperty;
class UClass;
class USceneComponent;

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_EquipmentInstance::UUR_EquipmentInstance(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

UWorld* UUR_EquipmentInstance::GetWorld() const
{
    if (APawn* OwningPawn = GetPawn())
    {
        return OwningPawn->GetWorld();
    }
    else
    {
        return nullptr;
    }
}

void UUR_EquipmentInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, Instigator);
    DOREPLIFETIME(ThisClass, SpawnedActors);
}

#if UE_WITH_IRIS
void UUR_EquipmentInstance::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
    using namespace UE::Net;

    // Build descriptors and allocate PropertyReplicationFragments for this object
    FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif // UE_WITH_IRIS

APawn* UUR_EquipmentInstance::GetPawn() const
{
    return Cast<APawn>(GetOuter());
}

APawn* UUR_EquipmentInstance::GetTypedPawn(TSubclassOf<APawn> PawnType) const
{
    APawn* Result = nullptr;
    if (UClass* ActualPawnType = PawnType)
    {
        if (GetOuter()->IsA(ActualPawnType))
        {
            Result = Cast<APawn>(GetOuter());
        }
    }
    return Result;
}

void UUR_EquipmentInstance::SpawnEquipmentActors(const TArray<FUR_EquipmentActorToSpawn>& ActorsToSpawn)
{
    if (APawn* OwningPawn = GetPawn())
    {
        USceneComponent* AttachTarget = OwningPawn->GetRootComponent();
        if (ACharacter* Char = Cast<ACharacter>(OwningPawn))
        {
            AttachTarget = Char->GetMesh();
        }

        for (const FUR_EquipmentActorToSpawn& SpawnInfo : ActorsToSpawn)
        {
            AActor* NewActor = GetWorld()->SpawnActorDeferred<AActor>(SpawnInfo.ActorToSpawn, FTransform::Identity, OwningPawn);
            NewActor->FinishSpawning(FTransform::Identity, /*bIsDefaultTransform=*/ true);
            NewActor->SetActorRelativeTransform(SpawnInfo.AttachTransform);
            NewActor->AttachToComponent(AttachTarget, FAttachmentTransformRules::KeepRelativeTransform, SpawnInfo.AttachSocket);

            SpawnedActors.Add(NewActor);
        }
    }
}

void UUR_EquipmentInstance::DestroyEquipmentActors()
{
    for (AActor* Actor : SpawnedActors)
    {
        if (Actor)
        {
            Actor->Destroy();
        }
    }
}

void UUR_EquipmentInstance::OnEquipped()
{
    K2_OnEquipped();
}

void UUR_EquipmentInstance::OnUnequipped()
{
    K2_OnUnequipped();
}

void UUR_EquipmentInstance::OnRep_Instigator()
{
}
