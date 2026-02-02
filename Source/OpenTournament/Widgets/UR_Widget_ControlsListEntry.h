// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"

#include "UR_Widget_ControlsListEntry.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UInputKeySelector;
class UTextBlock;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
UCLASS()
class OPENTOURNAMENT_API UUR_Widget_ControlsListEntry : public UUserWidget, public IUserObjectListEntry
{
    GENERATED_BODY()

public:
    UUR_Widget_ControlsListEntry(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;

    virtual void NativeOnListItemObjectSet(UObject* InObject) override;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* EntryName;

    UPROPERTY(meta = (BindWidget))
    UInputKeySelector* EntryKeySelector;

    UPROPERTY()
    TWeakObjectPtr<UObject> Item;

    UFUNCTION()
    void OnEntryKeySelectorKeyChanged(FInputChord SelectedKey);

    UFUNCTION()
    void OnEntryKeySelectorIsSelectingKeyChanged();

private:
    void UpdateEntry() const;

    bool IsSelectingKey;
};
