// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UObject/UObjectBase.h"

#include "GameplayTagContainer.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/WeakObjectPtr.h"

#include "UR_ContextEffectsLibrary.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

#define UE_API OPENTOURNAMENT_API

class UNiagaraSystem;
class USoundBase;
struct FFrame;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
UENUM()
enum class EContextEffectsLibraryLoadState : uint8
{
    Unloaded = 0,
    Loading = 1,
    Loaded = 2
};

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
USTRUCT(BlueprintType)
struct FGameContextEffects
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTag EffectTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTagContainer Context;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowedClasses = "/Script/Engine.SoundBase, /Script/Niagara.NiagaraSystem"))
    TArray<FSoftObjectPath> Effects;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
UCLASS(MinimalAPI)
class UUR_ActiveContextEffects : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere)
    FGameplayTag EffectTag;

    UPROPERTY(VisibleAnywhere)
    FGameplayTagContainer Context;

    UPROPERTY(VisibleAnywhere)
    TArray<TObjectPtr<USoundBase>> Sounds;

    UPROPERTY(VisibleAnywhere)
    TArray<TObjectPtr<UNiagaraSystem>> NiagaraSystems;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

DECLARE_DYNAMIC_DELEGATE_OneParam(FGameContextEffectLibraryLoadingComplete, TArray<UUR_ActiveContextEffects*>, GameActiveContextEffects);

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
UCLASS(MinimalAPI, BlueprintType)
class UUR_ContextEffectsLibrary : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FGameContextEffects> ContextEffects;

    UFUNCTION(BlueprintCallable)
    UE_API void GetEffects(const FGameplayTag Effect, const FGameplayTagContainer Context, TArray<USoundBase*>& Sounds, TArray<UNiagaraSystem*>& NiagaraSystems);

    UFUNCTION(BlueprintCallable)
    UE_API void LoadEffects();

    UE_API EContextEffectsLibraryLoadState GetContextEffectsLibraryLoadState() const;

private:
    bool IsEffectTagValidAndMatching(FGameplayTag Effect, const FGameplayTagContainer& Context, const TObjectPtr<UUR_ActiveContextEffects>& ActiveContextEffect) const;

    void LoadEffectsInternal();

    void GameContextEffectLibraryLoadingComplete(TArray<UUR_ActiveContextEffects*> GameActiveContextEffects);

    UPROPERTY(Transient)
    TArray<TObjectPtr<UUR_ActiveContextEffects>> ActiveContextEffects;

    UPROPERTY(Transient)
    EContextEffectsLibraryLoadState EffectsLoadState = EContextEffectsLibraryLoadState::Unloaded;
};

#undef UE_API
