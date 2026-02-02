// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "Cosmetics/UR_PawnComponent_CharacterParts.h"

#include "GameplayTagAssetInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

#include "Cosmetics/UR_CharacterPartTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_PawnComponent_CharacterParts)

/////////////////////////////////////////////////////////////////////////////////////////////////

class FLifetimeProperty;
class UPhysicsAsset;
class USkeletalMesh;
class UWorld;

/////////////////////////////////////////////////////////////////////////////////////////////////

FString FUR_AppliedCharacterPartEntry::GetDebugString() const
{
    return FString::Printf(TEXT("(PartClass: %s, Socket: %s, Instance: %s)"), *GetPathNameSafe(Part.PartClass), *Part.SocketName.ToString(), *GetPathNameSafe(SpawnedComponent));
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void FUR_CharacterPartList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
    bool bDestroyedAnyActors = false;
    for (int32 Index : RemovedIndices)
    {
        FUR_AppliedCharacterPartEntry& Entry = Entries[Index];
        bDestroyedAnyActors |= DestroyActorForEntry(Entry);
    }

    if (bDestroyedAnyActors && ensure(OwnerComponent))
    {
        OwnerComponent->BroadcastChanged();
    }
}

void FUR_CharacterPartList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
    bool bCreatedAnyActors = false;
    for (int32 Index : AddedIndices)
    {
        FUR_AppliedCharacterPartEntry& Entry = Entries[Index];
        bCreatedAnyActors |= SpawnActorForEntry(Entry);
    }

    if (bCreatedAnyActors && ensure(OwnerComponent))
    {
        OwnerComponent->BroadcastChanged();
    }
}

void FUR_CharacterPartList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
    bool bChangedAnyActors = false;

    // We don't support dealing with propagating changes, just destroy and recreate
    for (int32 Index : ChangedIndices)
    {
        FUR_AppliedCharacterPartEntry& Entry = Entries[Index];

        bChangedAnyActors |= DestroyActorForEntry(Entry);
        bChangedAnyActors |= SpawnActorForEntry(Entry);
    }

    if (bChangedAnyActors && ensure(OwnerComponent))
    {
        OwnerComponent->BroadcastChanged();
    }
}

FUR_CharacterPartHandle FUR_CharacterPartList::AddEntry(FUR_CharacterPart NewPart)
{
    FUR_CharacterPartHandle Result;
    Result.PartHandle = PartHandleCounter++;

    if (ensure(OwnerComponent && OwnerComponent->GetOwner() && OwnerComponent->GetOwner()->HasAuthority()))
    {
        FUR_AppliedCharacterPartEntry& NewEntry = Entries.AddDefaulted_GetRef();
        NewEntry.Part = NewPart;
        NewEntry.PartHandle = Result.PartHandle;

        if (SpawnActorForEntry(NewEntry))
        {
            OwnerComponent->BroadcastChanged();
        }

        MarkItemDirty(NewEntry);
    }

    return Result;
}

void FUR_CharacterPartList::RemoveEntry(FUR_CharacterPartHandle Handle)
{
    for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
    {
        FUR_AppliedCharacterPartEntry& Entry = *EntryIt;
        if (Entry.PartHandle == Handle.PartHandle)
        {
            const bool bDestroyedActor = DestroyActorForEntry(Entry);
            EntryIt.RemoveCurrent();
            MarkArrayDirty();

            if (bDestroyedActor && ensure(OwnerComponent))
            {
                OwnerComponent->BroadcastChanged();
            }

            break;
        }
    }
}

void FUR_CharacterPartList::ClearAllEntries(bool bBroadcastChangeDelegate)
{
    bool bDestroyedAnyActors = false;
    for (FUR_AppliedCharacterPartEntry& Entry : Entries)
    {
        bDestroyedAnyActors |= DestroyActorForEntry(Entry);
    }
    Entries.Reset();
    MarkArrayDirty();

    if (bDestroyedAnyActors && bBroadcastChangeDelegate && ensure(OwnerComponent))
    {
        OwnerComponent->BroadcastChanged();
    }
}

FGameplayTagContainer FUR_CharacterPartList::CollectCombinedTags() const
{
    FGameplayTagContainer Result;

    for (const FUR_AppliedCharacterPartEntry& Entry : Entries)
    {
        if (Entry.SpawnedComponent != nullptr)
        {
            if (IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(Entry.SpawnedComponent->GetChildActor()))
            {
                TagInterface->GetOwnedGameplayTags(/*inout*/ Result);
            }
        }
    }

    return Result;
}

bool FUR_CharacterPartList::SpawnActorForEntry(FUR_AppliedCharacterPartEntry& Entry)
{
    bool bCreatedAnyActors = false;

    if (ensure(OwnerComponent) && !OwnerComponent->IsNetMode(NM_DedicatedServer))
    {
        if (Entry.Part.PartClass != nullptr)
        {
            if (USceneComponent* ComponentToAttachTo = OwnerComponent->GetSceneComponentToAttachTo())
            {
                UChildActorComponent* PartComponent = NewObject<UChildActorComponent>(OwnerComponent->GetOwner());

                PartComponent->SetupAttachment(ComponentToAttachTo, Entry.Part.SocketName);
                PartComponent->SetChildActorClass(Entry.Part.PartClass);
                PartComponent->RegisterComponent();

                if (AActor* SpawnedActor = PartComponent->GetChildActor())
                {
                    switch (Entry.Part.CollisionMode)
                    {
                        case ECharacterCustomizationCollisionMode::UseCollisionFromCharacterPart:
                        {
                            // Do nothing
                            break;
                        }
                        case ECharacterCustomizationCollisionMode::NoCollision:
                        {
                            SpawnedActor->SetActorEnableCollision(false);
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }

                    // Set up a direct tick dependency to work around the child actor component not providing one
                    if (USceneComponent* SpawnedRootComponent = SpawnedActor->GetRootComponent())
                    {
                        SpawnedRootComponent->AddTickPrerequisiteComponent(ComponentToAttachTo);
                    }
                }

                Entry.SpawnedComponent = PartComponent;
                bCreatedAnyActors = true;
            }
        }
    }

    return bCreatedAnyActors;
}

bool FUR_CharacterPartList::DestroyActorForEntry(FUR_AppliedCharacterPartEntry& Entry)
{
    bool bDestroyedAnyActors = false;

    if (Entry.SpawnedComponent != nullptr)
    {
        Entry.SpawnedComponent->DestroyComponent();
        Entry.SpawnedComponent = nullptr;
        bDestroyedAnyActors = true;
    }

    return bDestroyedAnyActors;
}

//////////////////////////////////////////////////////////////////////

UUR_PawnComponent_CharacterParts::UUR_PawnComponent_CharacterParts(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetIsReplicatedByDefault(true);
}

void UUR_PawnComponent_CharacterParts::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, CharacterPartList);
}

FUR_CharacterPartHandle UUR_PawnComponent_CharacterParts::AddCharacterPart(const FUR_CharacterPart& NewPart)
{
    return CharacterPartList.AddEntry(NewPart);
}

void UUR_PawnComponent_CharacterParts::RemoveCharacterPart(FUR_CharacterPartHandle Handle)
{
    CharacterPartList.RemoveEntry(Handle);
}

void UUR_PawnComponent_CharacterParts::RemoveAllCharacterParts()
{
    CharacterPartList.ClearAllEntries(/*bBroadcastChangeDelegate=*/ true);
}

void UUR_PawnComponent_CharacterParts::BeginPlay()
{
    Super::BeginPlay();
}

void UUR_PawnComponent_CharacterParts::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    CharacterPartList.ClearAllEntries(/*bBroadcastChangeDelegate=*/ false);

    Super::EndPlay(EndPlayReason);
}

void UUR_PawnComponent_CharacterParts::OnRegister()
{
    Super::OnRegister();

    if (!IsTemplate())
    {
        CharacterPartList.SetOwnerComponent(this);
    }
}

TArray<AActor*> UUR_PawnComponent_CharacterParts::GetCharacterPartActors() const
{
    TArray<AActor*> Result;
    Result.Reserve(CharacterPartList.Entries.Num());

    for (const FUR_AppliedCharacterPartEntry& Entry : CharacterPartList.Entries)
    {
        if (UChildActorComponent* PartComponent = Entry.SpawnedComponent)
        {
            if (AActor* SpawnedActor = PartComponent->GetChildActor())
            {
                Result.Add(SpawnedActor);
            }
        }
    }

    return Result;
}

USkeletalMeshComponent* UUR_PawnComponent_CharacterParts::GetParentMeshComponent() const
{
    if (AActor* OwnerActor = GetOwner())
    {
        if (ACharacter* OwningCharacter = Cast<ACharacter>(OwnerActor))
        {
            if (USkeletalMeshComponent* MeshComponent = OwningCharacter->GetMesh())
            {
                return MeshComponent;
            }
        }
    }

    return nullptr;
}

USceneComponent* UUR_PawnComponent_CharacterParts::GetSceneComponentToAttachTo() const
{
    if (USkeletalMeshComponent* MeshComponent = GetParentMeshComponent())
    {
        return MeshComponent;
    }
    else if (AActor* OwnerActor = GetOwner())
    {
        return OwnerActor->GetRootComponent();
    }
    else
    {
        return nullptr;
    }
}

FGameplayTagContainer UUR_PawnComponent_CharacterParts::GetCombinedTags(FGameplayTag RequiredPrefix) const
{
    FGameplayTagContainer Result = CharacterPartList.CollectCombinedTags();
    if (RequiredPrefix.IsValid())
    {
        return Result.Filter(FGameplayTagContainer(RequiredPrefix));
    }
    else
    {
        return Result;
    }
}

void UUR_PawnComponent_CharacterParts::BroadcastChanged()
{
    // Check to see if the body type has changed
    if (USkeletalMeshComponent* MeshComponent = GetParentMeshComponent())
    {
        constexpr bool bReinitPose = true;

        // Determine the mesh to use based on cosmetic part tags
        const FGameplayTagContainer MergedTags = GetCombinedTags(FGameplayTag());
        USkeletalMesh* DesiredMesh = BodyMeshes.SelectBestBodyStyle(MergedTags);

        // Apply the desired mesh (this call is a no-op if the mesh hasn't changed)
        MeshComponent->SetSkeletalMesh(DesiredMesh, /*bReinitPose=*/ bReinitPose);

        // Apply the desired physics asset if there's a forced override independent of the one from the mesh
        if (UPhysicsAsset* PhysicsAsset = BodyMeshes.ForcedPhysicsAsset)
        {
            MeshComponent->SetPhysicsAsset(PhysicsAsset, /*bForceReInit=*/ bReinitPose);
        }
    }

    // Let observers know, e.g., if they need to apply team coloring or similar
    OnCharacterPartsChanged.Broadcast(this);
}
