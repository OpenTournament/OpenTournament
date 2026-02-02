// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"

#include "PocketCapture.generated.h"

#define UE_API POCKETWORLDS_API

enum ESceneCaptureSource : int;

class UMaterialInterface;
class UPocketCaptureSubsystem;
class UPrimitiveComponent;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class UWorld;
struct FFrame;

UCLASS(MinimalAPI, Abstract, Within=PocketCaptureSubsystem, BlueprintType, Blueprintable)
class UPocketCapture : public UObject
{
	GENERATED_BODY()

public:
	UE_API UPocketCapture();

	UE_API virtual void Initialize(UWorld* InWorld, int32 RendererIndex);
	UE_API virtual void Deinitialize();

	UE_API virtual void BeginDestroy() override;

	UFUNCTION(BlueprintCallable)
	UE_API void SetRenderTargetSize(int32 Width, int32 Height);

	UFUNCTION(BlueprintCallable)
	UE_API UTextureRenderTarget2D* GetOrCreateDiffuseRenderTarget();

	UFUNCTION(BlueprintCallable)
	UE_API UTextureRenderTarget2D* GetOrCreateAlphaMaskRenderTarget();

	UFUNCTION(BlueprintCallable)
	UE_API UTextureRenderTarget2D* GetOrCreateEffectsRenderTarget();

	UFUNCTION(BlueprintCallable)
	UE_API void SetCaptureTarget(AActor* InCaptureTarget);

	UFUNCTION(BlueprintCallable)
	UE_API void SetAlphaMaskedActors(const TArray<AActor*>& InCaptureTarget);

	UFUNCTION(BlueprintCallable)
	UE_API void CaptureDiffuse();

	UFUNCTION(BlueprintCallable)
	UE_API void CaptureAlphaMask();

	UFUNCTION(BlueprintCallable)
	UE_API void CaptureEffects();

	UFUNCTION(BlueprintCallable)
	UE_API virtual void ReleaseResources();

	UFUNCTION(BlueprintCallable)
	UE_API virtual void ReclaimResources();

	UFUNCTION(BlueprintCallable)
	UE_API int32 GetRendererIndex() const;
	
protected:
	AActor* GetCaptureTarget() const { return CaptureTargetPtr.Get(); }
	virtual void OnCaptureTargetChanged(AActor* InCaptureTarget) {}

	UE_API bool CaptureScene(UTextureRenderTarget2D* InRenderTarget, const TArray<AActor*>& InCaptureActors, ESceneCaptureSource CaptureSource, UMaterialInterface* OverrideMaterial);

protected:
	UE_API TArray<UPrimitiveComponent*> GatherPrimitivesForCapture(const TArray<AActor*>& InCaptureActors) const;
	
	UE_API UPocketCaptureSubsystem* GetThumbnailSystem() const;

protected:

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UMaterialInterface> AlphaMaskMaterial;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UMaterialInterface> EffectMaskMaterial;

protected:
	UPROPERTY(Transient)
	TObjectPtr<UWorld> PrivateWorld;

	UPROPERTY(Transient)
	int32 RendererIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere)
	int32 SurfaceWidth = 1;

	UPROPERTY(VisibleAnywhere)
	int32 SurfaceHeight = 1;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTextureRenderTarget2D> DiffuseRT;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTextureRenderTarget2D> AlphaMaskRT;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTextureRenderTarget2D> EffectsRT;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneCaptureComponent2D> CaptureComponent;

	UPROPERTY(VisibleAnywhere)
	TWeakObjectPtr<AActor> CaptureTargetPtr;

	UPROPERTY(VisibleAnywhere)
	TArray<TWeakObjectPtr<AActor>> AlphaMaskActorPtrs;
};

#undef UE_API
