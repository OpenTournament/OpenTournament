// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "GameFeatureAction_AddWidget.h"
#include "CommonUIExtensions.h"
#include "GameFeaturesSubsystemSettings.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/GameInstance.h"
#include "GameFeatures/GameFeatureAction_WorldActionBase.h"

#include "UR_HUD.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_AddWidget)

/////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "OTGameFeatures"

/////////////////////////////////////////////////////////////////////////////////////////////////
// UGameFeatureAction_AddWidgets

void UGameFeatureAction_AddWidgets::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
    Super::OnGameFeatureDeactivating(Context);

    FPerContextData* ActiveData = ContextData.Find(Context);
    if ensure(ActiveData)
    {
        Reset(*ActiveData);
    }
}

#if WITH_EDITORONLY_DATA
void UGameFeatureAction_AddWidgets::AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData)
{
    for (const FGameHUDElementEntry& Entry : Widgets)
    {
        AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateClient, Entry.WidgetClass.ToSoftObjectPath().GetAssetPath());
    }
}
#endif

#if WITH_EDITOR
EDataValidationResult UGameFeatureAction_AddWidgets::IsDataValid(FDataValidationContext& Context) const
{
    EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

    {
        int32 EntryIndex = 0;
        for (const FGameHUDLayoutRequest& Entry : Layout)
        {
            if (Entry.LayoutClass.IsNull())
            {
                Result = EDataValidationResult::Invalid;
                Context.AddError(FText::Format(LOCTEXT("LayoutHasNullClass", "Null WidgetClass at index {0} in Layout"), FText::AsNumber(EntryIndex)));
            }

            if (!Entry.LayerID.IsValid())
            {
                Result = EDataValidationResult::Invalid;
                Context.AddError(FText::Format(LOCTEXT("LayoutHasNoTag", "LayerID is not set at index {0} in Widgets"), FText::AsNumber(EntryIndex)));
            }

            ++EntryIndex;
        }
    }

    {
        int32 EntryIndex = 0;
        for (const FGameHUDElementEntry& Entry : Widgets)
        {
            if (Entry.WidgetClass.IsNull())
            {
                Result = EDataValidationResult::Invalid;
                Context.AddError(FText::Format(LOCTEXT("EntryHasNullClass", "Null WidgetClass at index {0} in Widgets"), FText::AsNumber(EntryIndex)));
            }

            if (!Entry.SlotID.IsValid())
            {
                Result = EDataValidationResult::Invalid;
                Context.AddError(FText::Format(LOCTEXT("EntryHasNoTag", "SlotID is not set at index {0} in Widgets"), FText::AsNumber(EntryIndex)));
            }
            ++EntryIndex;
        }
    }

    return Result;
}
#endif

void UGameFeatureAction_AddWidgets::AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
    UWorld* World = WorldContext.World();
    UGameInstance* GameInstance = WorldContext.OwningGameInstance;
    FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

    if ((GameInstance != nullptr) && (World != nullptr) && World->IsGameWorld())
    {
        if (UGameFrameworkComponentManager* ComponentManager = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
        {
            TSoftClassPtr<AActor> HUDActorClass = AUR_HUD::StaticClass();

            TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle = ComponentManager->AddExtensionHandler
            (
                HUDActorClass,
                UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::HandleActorExtension, ChangeContext));
            ActiveData.ComponentRequests.Add(ExtensionRequestHandle);
        }
    }
}

void UGameFeatureAction_AddWidgets::Reset(FPerContextData& ActiveData)
{
    ActiveData.ComponentRequests.Empty();

    for (TPair<FObjectKey, FPerActorData>& Pair : ActiveData.ActorData)
    {
        for (FUIExtensionHandle& Handle : Pair.Value.ExtensionHandles)
        {
            Handle.Unregister();
        }
    }
    ActiveData.ActorData.Empty();
}

void UGameFeatureAction_AddWidgets::HandleActorExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext)
{
    FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);
    if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved) || (EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved))
    {
        RemoveWidgets(Actor, ActiveData);
    }
    else if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded) || (EventName == UGameFrameworkComponentManager::NAME_GameActorReady))
    {
        AddWidgets(Actor, ActiveData);
    }
}

void UGameFeatureAction_AddWidgets::AddWidgets(AActor* Actor, FPerContextData& ActiveData)
{
    AUR_HUD* HUD = CastChecked<AUR_HUD>(Actor);

    if (!HUD->GetOwningPlayerController())
    {
        return;
    }

    if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(HUD->GetOwningPlayerController()->Player))
    {
        FPerActorData& ActorData = ActiveData.ActorData.FindOrAdd(HUD);

        for (const FGameHUDLayoutRequest& Entry : Layout)
        {
            if (TSubclassOf<UCommonActivatableWidget> ConcreteWidgetClass = Entry.LayoutClass.Get())
            {
                ActorData.LayoutsAdded.Add(UCommonUIExtensions::PushContentToLayer_ForPlayer(LocalPlayer, Entry.LayerID, ConcreteWidgetClass));
            }
        }

        UUIExtensionSubsystem* ExtensionSubsystem = HUD->GetWorld()->GetSubsystem<UUIExtensionSubsystem>();
        for (const FGameHUDElementEntry& Entry : Widgets)
        {
            ActorData.ExtensionHandles.Add(ExtensionSubsystem->RegisterExtensionAsWidgetForContext(Entry.SlotID, LocalPlayer, Entry.WidgetClass.Get(), -1));
        }
    }
}

void UGameFeatureAction_AddWidgets::RemoveWidgets(AActor* Actor, FPerContextData& ActiveData)
{
    AUR_HUD* HUD = CastChecked<AUR_HUD>(Actor);

    // Only unregister if this is the same HUD actor that was registered, there can be multiple active at once on the client
    FPerActorData* ActorData = ActiveData.ActorData.Find(HUD);

    if (ActorData)
    {
        for (TWeakObjectPtr<UCommonActivatableWidget>& AddedLayout : ActorData->LayoutsAdded)
        {
            if (AddedLayout.IsValid())
            {
                AddedLayout->DeactivateWidget();
            }
        }

        for (FUIExtensionHandle& Handle : ActorData->ExtensionHandles)
        {
            Handle.Unregister();
        }
        ActiveData.ActorData.Remove(HUD);
    }
}

#undef LOCTEXT_NAMESPACE
