// Copyright 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "OpenTournament.h"

#include "UR_GameModeBase.h"

#include "UR_FunctionLibrary.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations

class AUR_GameMode;

/////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * 
 */
UCLASS()
class OPENTOURNAMENT_API UUR_FunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    // Utility for retrieving GameMode CDO
    UFUNCTION(BlueprintPure, Category = "UnrealRemake|Character|State")
    static AUR_GameModeBase* GetGameModeDefaultObject(const UObject* WorldContextObject);

    UFUNCTION(BlueprintPure, Category = "UnrealRemake|Character|State") 
    static int32 GetPlayerStateValue(APlayerController* PlayerController);

    template<typename TEnum>
    static FORCEINLINE FString GetEnumValueAsString(const FString& Name, TEnum Value)
    {
        const UEnum* enumPtr = FindObject<UEnum>(ANY_PACKAGE, *Name, true);
        if (!enumPtr)
        {
            return FString("Invalid");
        }
        return enumPtr->GetNameByValue(static_cast<int64>(Value)).ToString();
    }
};
