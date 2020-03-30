// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerInput.h"
#include "UR_PlayerInput.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UInputSettings;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * 
 */
UCLASS(Config = Input)
class OPENTOURNAMENT_API UUR_PlayerInput : public UPlayerInput
{
    GENERATED_BODY()
    
    /////////////////////////////////////////////////////////////////////////////////////////////////

public:

    virtual void PostInitProperties() override;

    /////////////////////////////////////////////////////////////////////////////////////////////////

    void SetupActionMappings();

    UFUNCTION(BlueprintCallable, Category="UR_PlayerInput")
    void SetActionKeyMappingKey(const FInputActionKeyMapping& ActionKeyMapping, const FKey& Key);

    UFUNCTION(BlueprintCallable, Category = "UR_PlayerInput")
    void ModifyActionKeyMapping(const FName& ActionName, const FInputActionKeyMapping& ModActionKeyMapping);

    UFUNCTION(BlueprintCallable, Category = "UR_PlayerInput")
    void SaveMappings();

    /**
    * Modify a key mapping for an action or axis
    * returns true if the key mapping was modified successfully, otherwise returns false
    */
    bool ModifyKeyMapping(const FName& MappingName, const FInputChord& InputChord);


    UPROPERTY(Config)
    TArray<struct FInputActionKeyMapping> PlayerActionMappings;

private:

    void RemapAction(FInputActionKeyMapping ActionKeyMapping, const FKey& Key);
    void RemapAxis(FInputAxisKeyMapping AxisKeyMapping, const FKey& Key);

    UPROPERTY()
    UInputSettings* InputSettings;

};
