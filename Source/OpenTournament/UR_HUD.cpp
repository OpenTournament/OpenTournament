// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_HUD.h"

#include <AbilitySystemComponent.h>
#include <AbilitySystemGlobals.h>
#include <Blueprint/UserWidget.h>
#include <Blueprint/WidgetBlueprintLibrary.h>
#include <Components/GameFrameworkComponentManager.h>
#include <UObject/UObjectIterator.h>

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_HUD)

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_HUD::AUR_HUD(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bStartWithTickEnabled = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void AUR_HUD::Client_RestartHUD_Implementation()
{
    OnHUDRestart();
}

void AUR_HUD::PreInitializeComponents()
{
    Super::PreInitializeComponents();

    UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void AUR_HUD::BeginPlay()
{
    UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);

    Super::BeginPlay();
}

void AUR_HUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);

    Super::EndPlay(EndPlayReason);
}

void AUR_HUD::GetDebugActorList(TArray<AActor*>& InOutList)
{
    UWorld* World = GetWorld();

    Super::GetDebugActorList(InOutList);

    // Add all actors with an ability system component.
    for (TObjectIterator<UAbilitySystemComponent> It; It; ++It)
    {
        if (UAbilitySystemComponent* ASC = *It)
        {
            if (!ASC->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
            {
                AActor* AvatarActor = ASC->GetAvatarActor();
                AActor* OwnerActor = ASC->GetOwnerActor();

                if (AvatarActor && UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(AvatarActor))
                {
                    AddActorToDebugList(AvatarActor, InOutList, World);
                }
                else if (OwnerActor && UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerActor))
                {
                    AddActorToDebugList(OwnerActor, InOutList, World);
                }
            }
        }
    }
}

void AUR_HUD::ShowHUD()
{
    Super::ShowHUD();

    TArray<UUserWidget*> WidgetsToHide;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, WidgetsToHide, UUserWidget::StaticClass(), false);

    for (UUserWidget* Widget : WidgetsToHide)
    {
        if (Widget && Widget->IsInViewport())
        {
            // This will probably screw up any widgets that might actually be interactable,
            // but this should only be relevant for HUD
            auto VisibilityState = bShowHUD ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden;
            Widget->SetVisibility(VisibilityState);
        }
    }
}
