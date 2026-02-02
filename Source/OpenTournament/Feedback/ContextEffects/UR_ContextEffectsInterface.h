// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UObject/Interface.h"

#include <GameplayTagContainer.h>
#include <Engine/HitResult.h>

#include "UR_ContextEffectsInterface.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

#define UE_API OPENTOURNAMENT_API

class UAnimSequenceBase;
class UObject;
class USceneComponent;
struct FFrame;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
UENUM()
enum class EEffectsContextMatchType : uint8
{
    ExactMatch,
    BestMatch
};

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UUR_ContextEffectsInterface : public UInterface
{
    GENERATED_BODY()
};

class IUR_ContextEffectsInterface : public IInterface
{
    GENERATED_BODY()

public:
    /** */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    UE_API void AnimMotionEffect(const FName Bone
                                 , const FGameplayTag MotionEffect
                                 , USceneComponent* StaticMeshComponent
                                 , const FVector LocationOffset
                                 , const FRotator RotationOffset
                                 , const UAnimSequenceBase* AnimationSequence
                                 , const bool bHitSuccess
                                 , const FHitResult HitResult
                                 , FGameplayTagContainer Contexts
                                 , FVector VFXScale = FVector(1)
                                 , float AudioVolume = 1
                                 , float AudioPitch = 1);
};

#undef UE_API
