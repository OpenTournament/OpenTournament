// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "Widgets/UR_Widget_BaseMenu.h"

#include "UR_Widget_KeyBindingMenu.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class APlayerController;
class UButton;
class UListView;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Keybinding Menu
 */
UCLASS()
class OPENTOURNAMENT_API UUR_Widget_KeyBindingMenu : public UUR_Widget_BaseMenu
{
    GENERATED_BODY()
    
public:

    UUR_Widget_KeyBindingMenu(const FObjectInitializer& ObjectInitializer);

    void NativeOnInitialized() override;

    UPROPERTY(meta = (BindWidget))
    UButton* CloseButton;

    UPROPERTY(meta = (BindWidget))
    UListView* ControlsList;

    UFUNCTION()
    void OnCloseButtonClicked();

    UFUNCTION(BlueprintCallable, Category="UnrealRemake|UI")
    void OpenMenu();

private:

    void CreateKeyBindObject(FName Name, FKey Key);
    void PopulateKeyBindingList();

    UPROPERTY(EditDefaultsOnly)
    TArray<FName> AxisNames;

    UPROPERTY(EditDefaultsOnly)
    TArray<FName> ActionNames;

    UPROPERTY()
    APlayerController* OwningPlayer;
};
