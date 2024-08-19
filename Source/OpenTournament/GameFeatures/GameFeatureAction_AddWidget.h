// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CommonActivatableWidget.h"
#include "GameFeatureAction_WorldActionBase.h"
#include "UIExtensionSystem.h"

#include "GameFeatureAction_AddWidget.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

struct FWorldContext;
struct FComponentRequestHandle;

/////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct FGameHUDLayoutRequest
{
    GENERATED_BODY()

    // The layout widget to spawn
    UPROPERTY(EditAnywhere, Category=UI, meta=(AssetBundles="Client"))
    TSoftClassPtr<UCommonActivatableWidget> LayoutClass;

    // The layer to insert the widget in
    UPROPERTY(EditAnywhere, Category=UI, meta=(Categories="UI.Layer"))
    FGameplayTag LayerID;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct FGameHUDElementEntry
{
    GENERATED_BODY()

    // The widget to spawn
    UPROPERTY(EditAnywhere, Category=UI, meta=(AssetBundles="Client"))
    TSoftClassPtr<UUserWidget> WidgetClass;

    // The slot ID where we should place this widget
    UPROPERTY(EditAnywhere, Category = UI)
    FGameplayTag SlotID;
};

/////////////////////////////////////////////////////////////////////////////////////////////////
// UGameFeatureAction_AddWidget

/**
 * GameFeatureAction responsible for granting abilities (and attributes) to actors of a specified type.
 */
UCLASS(MinimalAPI, meta = (DisplayName = "Add Widgets"))
class UGameFeatureAction_AddWidgets final : public UGameFeatureAction_WorldActionBase
{
    GENERATED_BODY()

public:
    //~ Begin UGameFeatureAction interface
    virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
#if WITH_EDITORONLY_DATA
    virtual void AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData) override;
#endif
    //~ End UGameFeatureAction interface

    //~ Begin UObject interface
#if WITH_EDITOR
    virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
    //~ End UObject interface

private:
    // Layout to add to the HUD
    UPROPERTY(EditAnywhere, Category=UI, meta=(TitleProperty="{LayerID} -> {LayoutClass}"))
    TArray<FGameHUDLayoutRequest> Layout;

    // Widgets to add to the HUD
    UPROPERTY(EditAnywhere, Category=UI, meta=(TitleProperty="{SlotID} -> {WidgetClass}"))
    TArray<FGameHUDElementEntry> Widgets;

private:
    struct FPerActorData
    {
        TArray<TWeakObjectPtr<UCommonActivatableWidget>> LayoutsAdded;
        TArray<FUIExtensionHandle> ExtensionHandles;
    };

    struct FPerContextData
    {
        TArray<TSharedPtr<FComponentRequestHandle>> ComponentRequests;
        TMap<FObjectKey, FPerActorData> ActorData;
    };

    TMap<FGameFeatureStateChangeContext, FPerContextData> ContextData;

    //~ Begin UGameFeatureAction_WorldActionBase interface
    virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;

    //~ End UGameFeatureAction_WorldActionBase interface

    void Reset(FPerContextData& ActiveData);

    void HandleActorExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext);

    void AddWidgets(AActor* Actor, FPerContextData& ActiveData);

    void RemoveWidgets(AActor* Actor, FPerContextData& ActiveData);
};
