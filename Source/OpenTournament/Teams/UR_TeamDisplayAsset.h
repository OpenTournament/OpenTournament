// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Engine/DataAsset.h"

#include "UR_TeamDisplayAsset.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

struct FPropertyChangedEvent;

class UMaterialInstanceDynamic;
class UMeshComponent;
class UNiagaraComponent;
class AActor;
class UTexture;

/////////////////////////////////////////////////////////////////////////////////////////////////

// Represents the display information for team definitions (e.g., colors, display names, textures, etc...)
UCLASS(BlueprintType)
class UUR_TeamDisplayAsset : public UDataAsset
{
    GENERATED_BODY()

public:

    UUR_TeamDisplayAsset(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

#if WITH_EDITOR
    virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif // WITH_EDITOR

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TMap<FName, float> ScalarParameters;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TMap<FName, FLinearColor> ColorParameters;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TMap<FName, TObjectPtr<UTexture>> TextureParameters;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText TeamShortName;

public:
    UFUNCTION(BlueprintCallable, Category=Teams)
    void ApplyToMaterial(UMaterialInstanceDynamic* Material);

    UFUNCTION(BlueprintCallable, Category=Teams)
    void ApplyToMeshComponent(UMeshComponent* MeshComponent);

    UFUNCTION(BlueprintCallable, Category=Teams)
    void ApplyToNiagaraComponent(UNiagaraComponent* NiagaraComponent);

    UFUNCTION(BlueprintCallable, Category=Teams, meta=(DefaultToSelf="TargetActor"))
    void ApplyToActor(AActor* TargetActor, bool bIncludeChildActors = true);

public:
    //~UObject interface
#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
    //~End of UObject interface
};
