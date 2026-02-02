// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "Templates/SubclassOf.h"

#include "UR_DevelopmentStatics.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UClass;
class UObject;
class UWorld;
struct FAssetData;
struct FFrame;

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class UUR_DevelopmentStatics : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // Should game logic skip directly to gameplay (skipping any match warmup / waiting for players / etc... aspects)
    // Will always return false except when playing in the editor and bTestFullGameFlowInPIE (in OT Developer Settings) is false
    UFUNCTION(BlueprintCallable, Category="OT")
    static bool ShouldSkipDirectlyToGameplay();

    // Should game logic load cosmetic backgrounds in the editor?
    // Will always return true except when playing in the editor and bSkipLoadingCosmeticBackgroundsInPIE (in OT Developer Settings) is true
    UFUNCTION(BlueprintCallable, Category = "OT", meta=(ExpandBoolAsExecs="ReturnValue"))
    static bool ShouldLoadCosmeticBackgrounds();

    // Should game logic load cosmetic backgrounds in the editor?
    // Will always return true except when playing in the editor and bSkipLoadingCosmeticBackgroundsInPIE (in OT Developer Settings) is true
    UFUNCTION(BlueprintCallable, Category = "OT")
    static bool CanPlayerBotsAttack();

    // Finds the most appropriate play-in-editor world to run 'server' cheats on
    //   This might be the only world if running standalone, the listen server, or the dedicated server
    static UWorld* FindPlayInEditorAuthorityWorld();

    // Tries to find a class by a short name (with some heuristics to improve the usability when done via a cheat console)
    static UClass* FindClassByShortName(const FString& SearchToken, UClass* DesiredBaseClass, bool bLogFailures = true);

    template <typename DesiredClass>
    static TSubclassOf<DesiredClass> FindClassByShortName(const FString& SearchToken, bool bLogFailures = true)
    {
        return FindClassByShortName(SearchToken, DesiredClass::StaticClass(), bLogFailures);
    }

private:
    static TArray<FAssetData> GetAllBlueprints();
    static UClass* FindBlueprintClass(const FString& TargetNameRaw, UClass* DesiredBaseClass);
};
