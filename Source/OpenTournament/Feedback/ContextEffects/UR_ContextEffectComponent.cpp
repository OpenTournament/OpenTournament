// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_ContextEffectComponent.h"

#include "Engine/World.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

#include "UR_ContextEffectsSettings.h"
#include "UR_ContextEffectsSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_ContextEffectComponent)

/////////////////////////////////////////////////////////////////////////////////////////////////

class UAnimSequenceBase;
class USceneComponent;

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_ContextEffectComponent::UUR_ContextEffectComponent()
{
    // Disable component tick, enable Auto Activate
    PrimaryComponentTick.bCanEverTick = false;
    bAutoActivate = true;
}

void UUR_ContextEffectComponent::BeginPlay()
{
    Super::BeginPlay();

    CurrentContexts.AppendTags(DefaultEffectContexts);
    CurrentContextEffectsLibraries = DefaultContextEffectsLibraries;

    // On Begin Play, Load and Add Context Effects pairings
    if (const UWorld* World = GetWorld())
    {
        if (UUR_ContextEffectsSubsystem* ContextEffectsSubsystem = World->GetSubsystem<UUR_ContextEffectsSubsystem>())
        {
            ContextEffectsSubsystem->LoadAndAddContextEffectsLibraries(GetOwner(), CurrentContextEffectsLibraries);
        }
    }
}

void UUR_ContextEffectComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // On End Play, remove unnecessary context effects pairings
    if (const UWorld* World = GetWorld())
    {
        if (UUR_ContextEffectsSubsystem* ContextEffectsSubsystem = World->GetSubsystem<UUR_ContextEffectsSubsystem>())
        {
            ContextEffectsSubsystem->UnloadAndRemoveContextEffectsLibraries(GetOwner());
        }
    }

    Super::EndPlay(EndPlayReason);
}

// Implementation of Interface's AnimMotionEffect function
void UUR_ContextEffectComponent::AnimMotionEffect_Implementation(const FName Bone, const FGameplayTag MotionEffect, USceneComponent* StaticMeshComponent,
                                                                 const FVector LocationOffset, const FRotator RotationOffset, const UAnimSequenceBase* AnimationSequence,
                                                                 const bool bHitSuccess, const FHitResult HitResult, FGameplayTagContainer Contexts,
                                                                 FVector VFXScale, float AudioVolume, float AudioPitch)
{
    // Prep Components
    TArray<UAudioComponent*> AudioComponentsToAdd;
    TArray<UNiagaraComponent*> NiagaraComponentsToAdd;

    FGameplayTagContainer TotalContexts;

    // Aggregate contexts
    TotalContexts.AppendTags(Contexts);
    TotalContexts.AppendTags(CurrentContexts);

    // Check if converting Physical Surface Type to Context
    if (bConvertPhysicalSurfaceToContext)
    {
        // Get Phys Mat Type Pointer
        TWeakObjectPtr<UPhysicalMaterial> PhysicalSurfaceTypePtr = HitResult.PhysMaterial;

        // Check if pointer is okay
        if (PhysicalSurfaceTypePtr.IsValid())
        {
            // Get the Surface Type Pointer
            TEnumAsByte<EPhysicalSurface> PhysicalSurfaceType = PhysicalSurfaceTypePtr->SurfaceType;

            // If Settings are valid
            if (const UUR_ContextEffectsSettings* ContextEffectsSettings = GetDefault<UUR_ContextEffectsSettings>())
            {
                // Convert Surface Type to known
                if (const FGameplayTag* SurfaceContextPtr = ContextEffectsSettings->SurfaceTypeToContextMap.Find(PhysicalSurfaceType))
                {
                    FGameplayTag SurfaceContext = *SurfaceContextPtr;

                    TotalContexts.AddTag(SurfaceContext);
                }
            }
        }
    }

    // Cycle through Active Audio Components and cache
    for (UAudioComponent* ActiveAudioComponent : ActiveAudioComponents)
    {
        if (ActiveAudioComponent)
        {
            AudioComponentsToAdd.Add(ActiveAudioComponent);
        }
    }

    // Cycle through Active Niagara Components and cache
    for (UNiagaraComponent* ActiveNiagaraComponent : ActiveNiagaraComponents)
    {
        if (ActiveNiagaraComponent)
        {
            NiagaraComponentsToAdd.Add(ActiveNiagaraComponent);
        }
    }

    // Get World
    if (const UWorld* World = GetWorld())
    {
        // Get Subsystem
        if (UUR_ContextEffectsSubsystem* ContextEffectsSubsystem = World->GetSubsystem<UUR_ContextEffectsSubsystem>())
        {
            // Set up Audio Components and Niagara
            TArray<UAudioComponent*> AudioComponents;
            TArray<UNiagaraComponent*> NiagaraComponents;

            // Spawn effects
            ContextEffectsSubsystem->SpawnContextEffects
            (GetOwner(),
                StaticMeshComponent,
                Bone,
                LocationOffset,
                RotationOffset,
                MotionEffect,
                TotalContexts,
                AudioComponents,
                NiagaraComponents,
                VFXScale,
                AudioVolume,
                AudioPitch);

            // Append resultant effects
            AudioComponentsToAdd.Append(AudioComponents);
            NiagaraComponentsToAdd.Append(NiagaraComponents);
        }
    }

    // Append Active Audio Components
    ActiveAudioComponents.Empty();
    ActiveAudioComponents.Append(AudioComponentsToAdd);

    // Append Active
    ActiveNiagaraComponents.Empty();
    ActiveNiagaraComponents.Append(NiagaraComponentsToAdd);
}

void UUR_ContextEffectComponent::UpdateEffectContexts(FGameplayTagContainer NewEffectContexts)
{
    // Reset and update
    CurrentContexts.Reset(NewEffectContexts.Num());
    CurrentContexts.AppendTags(NewEffectContexts);
}

void UUR_ContextEffectComponent::UpdateLibraries(const TSet<TSoftObjectPtr<UUR_ContextEffectsLibrary>> NewContextEffectsLibraries)
{
    // Clear out existing Effects
    CurrentContextEffectsLibraries = NewContextEffectsLibraries;

    if (const UWorld* World = GetWorld())
    {
        if (UUR_ContextEffectsSubsystem* ContextEffectsSubsystem = World->GetSubsystem<UUR_ContextEffectsSubsystem>())
        {
            // Load and Add Libraries to Subsystem
            ContextEffectsSubsystem->LoadAndAddContextEffectsLibraries(GetOwner(), CurrentContextEffectsLibraries);
        }
    }
}
