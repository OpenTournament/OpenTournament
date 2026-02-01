// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_GameplayAbilityWithWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_GameplayAbilityWithWidget)

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_GameplayAbilityWithWidget::UUR_GameplayAbilityWithWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{

}

void UUR_GameplayAbilityWithWidget::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
    if (WidgetInterfaceData.Num() > 0)
    {
        for (const auto& WidgetInterfaceEntry : WidgetInterfaceData)
        {
            if (UUIExtensionSubsystem* ExtensionSubsystem = GetWorld()->GetSubsystem<UUIExtensionSubsystem>())
            {
                auto OwningActor = GetOwningActorFromActorInfo();
                auto ExtensionHandle = ExtensionSubsystem->RegisterExtensionAsWidgetForContext(WidgetInterfaceEntry.ExtensionPointTag, OwningActor, WidgetInterfaceEntry.WidgetClass, WidgetInterfaceEntry.Priority);
                WidgetExtensionHandles.Add(ExtensionHandle);
            }
        }
    }
    Super::OnGiveAbility(ActorInfo, Spec);
}

void UUR_GameplayAbilityWithWidget::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
    for (auto& IterEntry : WidgetExtensionHandles)
    {
        IterEntry.Unregister();
    }
    WidgetExtensionHandles.Empty();

    Super::OnRemoveAbility(ActorInfo, Spec);
}
