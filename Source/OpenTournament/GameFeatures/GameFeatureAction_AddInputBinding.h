// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameFeatureAction_WorldActionBase.h"

#include "UObject/SoftObjectPtr.h"

#include "GameFeatureAction_AddInputBinding.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class AActor;
class UInputMappingContext;
class UPlayer;
class APlayerController;
struct FComponentRequestHandle;
class UUR_InputConfig;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Adds InputMappingContext to local players' EnhancedInput system.
 * Expects that local players are set up to use the EnhancedInput system.
 */
UCLASS(MinimalAPI, meta = (DisplayName = "Add Input Binds"))
class UGameFeatureAction_AddInputBinding final : public UGameFeatureAction_WorldActionBase
{
    GENERATED_BODY()

public:
    //~ Begin UGameFeatureAction interface
    virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
    virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
    //~ End UGameFeatureAction interface

    //~ Begin UObject interface
#if WITH_EDITOR
    virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
    //~ End UObject interface

    UPROPERTY(EditAnywhere, Category="Input", meta=(AssetBundles="Client,Server"))
    TArray<TSoftObjectPtr<const UUR_InputConfig>> InputConfigs;

private:
    struct FPerContextData
    {
        TArray<TSharedPtr<FComponentRequestHandle>> ExtensionRequestHandles;
        TArray<TWeakObjectPtr<APawn>> PawnsAddedTo;
    };

    TMap<FGameFeatureStateChangeContext, FPerContextData> ContextData;

    //~ Begin UGameFeatureAction_WorldActionBase interface
    virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;
    //~ End UGameFeatureAction_WorldActionBase interface

    void Reset(FPerContextData& ActiveData);
    void HandlePawnExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext);
    void AddInputMappingForPlayer(APawn* Pawn, FPerContextData& ActiveData);
    void RemoveInputMapping(APawn* Pawn, FPerContextData& ActiveData);
};
