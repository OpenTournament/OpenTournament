// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_ImpactDecalComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UR_FunctionLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_ImpactDecalComponent)

/////////////////////////////////////////////////////////////////////////////////////////////////

const FName& CreationTimeParamName = "CreationTime";

void UUR_ImpactDecalComponent::BeginPlay()
{
    USceneComponent::BeginPlay();

    CreationTime = GetWorld()->GetTimeSeconds();

    auto MID = Cast<UMaterialInstanceDynamic>(DecalMaterial);
    if (!MID && DecalMaterial)
    {
        TArray<FMaterialParameterInfo> Scalars;
        TArray<FGuid> Guids;
        DecalMaterial->GetAllScalarParameterInfo(Scalars, Guids);
        for (const auto& Scalar : Scalars)
        {
            if (Scalar.Name == CreationTimeParamName)
            {
                MID = CreateDynamicMaterialInstance();
                break;
            }
        }
    }
    if (MID)
    {
        MID->SetScalarParameterValue(CreationTimeParamName, CreationTime);
    }

    SetFadeOut(FadeStartDelay, FadeDuration, true);
}

UUR_ImpactDecalComponent* CreateImpactDecalComponent(class UMaterialInterface* DecalMaterial, const FVector& DecalSize, UWorld* World, AActor* Actor)
{
    if (World && World->GetNetMode() == NM_DedicatedServer)
    {
        return nullptr;
    }

    auto DecalComp = NewObject<UUR_ImpactDecalComponent>(Actor ? Actor : (UObject*)World);
    if (IsValid(DecalComp))
    {
        DecalComp->bAllowAnyoneToDestroyMe = true;
        DecalComp->SetDecalMaterial(DecalMaterial);
        DecalComp->DecalSize = DecalSize;
        DecalComp->SetUsingAbsoluteScale(true);
        DecalComp->RegisterComponentWithWorld(World);
    }

    return DecalComp;
}

UUR_ImpactDecalComponent* UUR_ImpactDecalComponent::SpawnImpactDecal(const UObject* WorldContext, class UMaterialInterface* Material, const FVector& Location, const FVector& Direction, FVector Size)
{
    if (Material)
    {
        if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
        {
            if (auto DecalComp = CreateImpactDecalComponent(Material, Size, World, (AActor*)World->GetWorldSettings()))
            {
                DecalComp->SetWorldLocationAndRotation(Location, Direction.Rotation());
                return DecalComp;
            }
        }
    }
    return nullptr;
}

UUR_ImpactDecalComponent* UUR_ImpactDecalComponent::SpawnImpactDecal2(const UObject* WorldContext, UMaterialInterface* Material, const FVector& HitLocation, const FVector& HitNormal, const FVector& ShotDirection, FVector Size, float MaxAngle)
{
    return SpawnImpactDecal(WorldContext, Material, HitLocation, -UUR_FunctionLibrary::ConstrainVectorInCone(-ShotDirection, HitNormal, MaxAngle), Size);
}
