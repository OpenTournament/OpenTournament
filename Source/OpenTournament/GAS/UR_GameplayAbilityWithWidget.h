// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UR_GameplayAbility.h"

#include <UIExtensionSystem.h>

#include "UR_GameplayAbilityWithWidget.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FGameplayAbilityWidgetInterfaceData
{
    GENERATED_BODY()

public:
    // WidgetClass to utilize
    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<UUserWidget> WidgetClass = nullptr;

    // Tag used to associate the ExtensionHandle
    UPROPERTY(EditDefaultsOnly)
    FGameplayTag ExtensionPointTag;

    UPROPERTY(EditDefaultsOnly)
    int32 Priority = -1;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Subclass of ability blueprint type with game-specific data
 *
 *
 */
UCLASS(Abstract, HideCategories = Input, Meta = (ShortTooltip = "Gameplay ability class with a Widget for Visualization."))
class OPENTOURNAMENT_API UUR_GameplayAbilityWithWidget : public UUR_GameplayAbility
{
    GENERATED_BODY()

public:
    UUR_GameplayAbilityWithWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

#pragma region UGameplayAbilityInterface

    virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
    virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

#pragma endregion // UGameplayAbilityInterface

    // Array of Structures containing Widget and UI-Extension Data
    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
    TArray<FGameplayAbilityWidgetInterfaceData> WidgetInterfaceData;

    // Array of Handles to UI Data
    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
    TArray<FUIExtensionHandle> WidgetExtensionHandles;
};
