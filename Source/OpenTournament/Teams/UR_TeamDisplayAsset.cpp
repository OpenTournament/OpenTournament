// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_TeamDisplayAsset.h"

#include <NiagaraComponent.h>
#include <Components/MeshComponent.h>
#include <Engine/Texture.h>
#include <Materials/MaterialInstanceDynamic.h>

#include "Teams/UR_TeamSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_TeamDisplayAsset)

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_TeamDisplayAsset::UUR_TeamDisplayAsset(const FObjectInitializer& ObjectInitializer)
{
    // Noop
}

/////////////////////////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
EDataValidationResult UUR_TeamDisplayAsset::IsDataValid(FDataValidationContext& Context) const
{
    return Super::IsDataValid(Context);
}
#endif //WITH_EDITOR

/////////////////////////////////////////////////////////////////////////////////////////////////

void UUR_TeamDisplayAsset::ApplyToMaterial(UMaterialInstanceDynamic* Material)
{
    if (Material)
    {
        for (const auto& KVP : ScalarParameters)
        {
            Material->SetScalarParameterValue(KVP.Key, KVP.Value);
        }

        for (const auto& KVP : ColorParameters)
        {
            Material->SetVectorParameterValue(KVP.Key, FVector(KVP.Value));
        }

        for (const auto& KVP : TextureParameters)
        {
            Material->SetTextureParameterValue(KVP.Key, KVP.Value);
        }
    }
}

void UUR_TeamDisplayAsset::ApplyToMeshComponent(UMeshComponent* MeshComponent)
{
    if (MeshComponent)
    {
        for (const auto& KVP : ScalarParameters)
        {
            MeshComponent->SetScalarParameterValueOnMaterials(KVP.Key, KVP.Value);
        }

        for (const auto& KVP : ColorParameters)
        {
            MeshComponent->SetVectorParameterValueOnMaterials(KVP.Key, FVector(KVP.Value));
        }

        const TArray<UMaterialInterface*> MaterialInterfaces = MeshComponent->GetMaterials();
        for (int32 MaterialIndex = 0; MaterialIndex < MaterialInterfaces.Num(); ++MaterialIndex)
        {
            if (UMaterialInterface* MaterialInterface = MaterialInterfaces[MaterialIndex])
            {
                UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(MaterialInterface);
                if (!DynamicMaterial)
                {
                    DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(MaterialIndex);
                }

                for (const auto& KVP : TextureParameters)
                {
                    DynamicMaterial->SetTextureParameterValue(KVP.Key, KVP.Value);
                }
            }
        }
    }
}

void UUR_TeamDisplayAsset::ApplyToNiagaraComponent(UNiagaraComponent* NiagaraComponent)
{
    if (NiagaraComponent)
    {
        for (const auto& KVP : ScalarParameters)
        {
            NiagaraComponent->SetVariableFloat(KVP.Key, KVP.Value);
        }

        for (const auto& KVP : ColorParameters)
        {
            NiagaraComponent->SetVariableLinearColor(KVP.Key, KVP.Value);
        }

        for (const auto& KVP : TextureParameters)
        {
            UTexture* Texture = KVP.Value;
            NiagaraComponent->SetVariableTexture(KVP.Key, Texture);
        }
    }
}

void UUR_TeamDisplayAsset::ApplyToActor(AActor* TargetActor, bool bIncludeChildActors)
{
    if (TargetActor != nullptr)
    {
        TargetActor->ForEachComponent(bIncludeChildActors,
            [this](UActorComponent* InComponent)
            {
                if (UMeshComponent* MeshComponent = Cast<UMeshComponent>(InComponent))
                {
                    ApplyToMeshComponent(MeshComponent);
                }
                else if (UNiagaraComponent* NiagaraComponent = Cast<UNiagaraComponent>(InComponent))
                {
                    ApplyToNiagaraComponent(NiagaraComponent);
                }
            });
    }
}

#if WITH_EDITOR
void UUR_TeamDisplayAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    for (UUR_TeamSubsystem* TeamSubsystem : TObjectRange<UUR_TeamSubsystem>())
    {
        TeamSubsystem->NotifyTeamDisplayAssetModified(this);
    }
}
#endif
