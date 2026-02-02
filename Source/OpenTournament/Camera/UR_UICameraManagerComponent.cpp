// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_UICameraManagerComponent.h"

#include "GameFramework/HUD.h"
#include "GameFramework/PlayerController.h"

#include "UR_PlayerCameraManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_UICameraManagerComponent)

/////////////////////////////////////////////////////////////////////////////////////////////////

class AActor;
class FDebugDisplayInfo;

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_UICameraManagerComponent* UUR_UICameraManagerComponent::GetComponent(APlayerController* PC)
{
    if (PC != nullptr)
    {
        if (AUR_PlayerCameraManager* PCCamera = Cast<AUR_PlayerCameraManager>(PC->PlayerCameraManager))
        {
            return PCCamera->GetUICameraComponent();
        }
    }

    return nullptr;
}

UUR_UICameraManagerComponent::UUR_UICameraManagerComponent()
{
    bWantsInitializeComponent = true;

    if (!HasAnyFlags(RF_ClassDefaultObject))
    {
        // Register "showdebug" hook.
        if (!IsRunningDedicatedServer())
        {
            AHUD::OnShowDebugInfo.AddUObject(this, &ThisClass::OnShowDebugInfo);
        }
    }
}

void UUR_UICameraManagerComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UUR_UICameraManagerComponent::SetViewTarget(AActor* InViewTarget, FViewTargetTransitionParams TransitionParams)
{
    TGuardValue<bool> UpdatingViewTargetGuard(bUpdatingViewTarget, true);

    ViewTarget = InViewTarget;
    CastChecked<AUR_PlayerCameraManager>(GetOwner())->SetViewTarget(ViewTarget, TransitionParams);
}

bool UUR_UICameraManagerComponent::NeedsToUpdateViewTarget() const
{
    return false;
}

void UUR_UICameraManagerComponent::UpdateViewTarget(struct FTViewTarget& OutVT, float DeltaTime)
{}

void UUR_UICameraManagerComponent::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos)
{}
