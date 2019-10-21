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
    UFUNCTION(BlueprintPure, Category = "Character|State")
    static AUR_GameModeBase* GetGameModeDefaultObject(const UObject* WorldContextObject);

    UFUNCTION(BlueprintPure, Category = "Character|State") 
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

	/**
	* Returns true if given key is mapped to given action.
	* Does not consider modifiers.
	*
	* This is a helper for doing special UMG navigation via OnKeyDown,
	* using realtime binds and not hardcoded.
	*/
	UFUNCTION(BlueprintPure)
	static bool IsKeyMappedToAction(const FKey& Key, FName ActionName);

	/**
	* Returns true if given key is mapped to given axis.
	* Use direction 0 for any.
	*
	* This is a helper for doing special UMG navigation via OnKeyDown,
	* using realtime binds and not hardcoded.
	*/
	UFUNCTION(BlueprintPure)
	static bool IsKeyMappedToAxis(const FKey& Key, FName AxisName, float Direction = 1.f);

};
