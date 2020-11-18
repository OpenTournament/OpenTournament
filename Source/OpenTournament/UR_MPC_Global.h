// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UR_MPC_Global.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

class UMaterialParameterCollection;

/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Wrapper class for manipulating the global-game MaterialParameterCollection.
 * Implements a parameter-mapping functionality so certain parameters can follow the value of other parameters.
 *
 * NOTE: Blueprints should avoid manipulating the collection directly with KismetMaterialLibrary nodes,
 * because it would break the automagic mappings.
 *
 * https://docs.unrealengine.com/en-US/Engine/Rendering/Materials/ParameterCollections/index.html
 */
UCLASS()
class OPENTOURNAMENT_API UUR_MPC_Global : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * SOME NOTES:
    *
    * I plan to give user ability to configure his own team color (AllyColor),
    * as well as the main enemy color (EnemyColor),
    * and up to 2 additional enemy colors for 4-team games (EnemyColor2, EnemyColor3).
    *
    * Gamemodes with huge amount of teams like BR will need a different handling,
    * such as showing all enemies with the main enemy color, like FFA.
    *
    * Parameters AllyColor and EnemyColor should always match user's settings.
    *
    * Parameters TeamColor_X will be updated in real-time according to user's team.
    * For example if user is in Team 1 :
    * - Team0 will be given EnemyColor
    * - Team1 will be given AllyColor
    * - Team2 will be given EnemyColor2
    * - Team3 will be given EnemyColor3
    *
    * Parameters CustomColor_X shall have different usages according to the current gamemode (DOM, Assault..)
    */

    /**
    * Ally color according to local player settings.
    */
    UPROPERTY(BlueprintReadOnly)
    FName P_AllyColor = FName(TEXT("AllyColor"));

    /**
    * Primary enemy color according to local player settings.
    */
    UPROPERTY(BlueprintReadOnly)
    FName P_EnemyColor = FName(TEXT("EnemyColor"));

    /**
    * Second enemy color according to local player settings.
    */
    UPROPERTY(BlueprintReadOnly)
    FName P_EnemyColor2 = FName(TEXT("EnemyColor2"));

    /**
    * Third enemy color according to local player settings.
    */
    UPROPERTY(BlueprintReadOnly)
    FName P_EnemyColor3 = FName(TEXT("EnemyColor3"));

    /**
    * Color associated with TeamIndex 0 according to local player's team.
    */
    UPROPERTY(BlueprintReadOnly)
    FName P_TeamColor0 = FName(TEXT("TeamColor0"));

    /**
    * Color associated with TeamIndex 1 according to local player's team.
    */
    UPROPERTY(BlueprintReadOnly)
    FName P_TeamColor1 = FName(TEXT("TeamColor1"));

    /**
    * Color associated with TeamIndex 2 according to local player's team.
    */
    UPROPERTY(BlueprintReadOnly)
    FName P_TeamColor2 = FName(TEXT("TeamColor2"));

    /**
    * Color associated with TeamIndex 3 according to local player's team.
    */
    UPROPERTY(BlueprintReadOnly)
    FName P_TeamColor3 = FName(TEXT("TeamColor3"));

    /**
    * The CustomColor_* are additional parameters that are set by the game.
    * They can be leveraged by level designers to colorize parts of a level area according to the team controlling it.
    *
    * They have different meanings depending on the gamemode.
    * For example :
    * - in DOM, CustomColor_X is associated to the team controlling point X.
    * - in Assault, CustomColor_0 and 1 are associated with attacking and defending teams respectively.
    */
    UPROPERTY(BlueprintReadOnly)
    FName P_CustomColor0 = FName(TEXT("CustomColor0"));
    UPROPERTY(BlueprintReadOnly)
    FName P_CustomColor1 = FName(TEXT("CustomColor1"));
    UPROPERTY(BlueprintReadOnly)
    FName P_CustomColor2 = FName(TEXT("CustomColor2"));
    UPROPERTY(BlueprintReadOnly)
    FName P_CustomColor3 = FName(TEXT("CustomColor3"));
    UPROPERTY(BlueprintReadOnly)
    FName P_CustomColor4 = FName(TEXT("CustomColor4"));
    UPROPERTY(BlueprintReadOnly)
    FName P_CustomColor5 = FName(TEXT("CustomColor5"));
    UPROPERTY(BlueprintReadOnly)
    FName P_CustomColor6 = FName(TEXT("CustomColor6"));
    UPROPERTY(BlueprintReadOnly)
    FName P_CustomColor7 = FName(TEXT("CustomColor7"));
    UPROPERTY(BlueprintReadOnly)
    FName P_CustomColor8 = FName(TEXT("CustomColor8"));
    UPROPERTY(BlueprintReadOnly)
    FName P_CustomColor9 = FName(TEXT("CustomColor9"));

    UFUNCTION(BlueprintPure, BlueprintCosmetic, Meta = (WorldContext = "WorldContext", DisplayName = "MPC_Global_GetParams", CompactNodeTitle = "MPC"))
    static UUR_MPC_Global* Get(UObject* WorldContext, bool bFallbackToCDO = false);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    UFUNCTION(BlueprintCallable, BlueprintCosmetic, Meta = (DisplayName = "MPC_Global_SetScalar", WorldContext = "WorldContext"))
    static void SetScalar(UObject* WorldContext, FName Param, float Value);

    UFUNCTION(BlueprintPure, BlueprintCosmetic, Meta = (DisplayName = "MPC_Global_GetScalar", WorldContext = "WorldContext"))
    static float GetScalar(UObject* WorldContext, FName Param);

    UFUNCTION(BlueprintCallable, BlueprintCosmetic, Meta = (DisplayName = "MPC_Global_SetVector", WorldContext = "WorldContext"))
    static void SetVector(UObject* WorldContext, FName Param, const FLinearColor& Value);

    UFUNCTION(BlueprintPure, BlueprintCosmetic, Meta = (DisplayName = "MPC_Global_GetVector", WorldContext = "WorldContext"))
    static FLinearColor GetVector(UObject* WorldContext, FName Param);

protected:

    static UMaterialParameterCollection* GetCollection(const UObject* WorldContext);

    /////////////////////////////////////////////////////////////////////////////////////////////////

public:

    UFUNCTION(BlueprintCallable, BlueprintCosmetic, Meta = (DisplayName = "MPC_Global_MapParameter", WorldContext = "WorldContext"))
    static bool MapParameter(UObject* WorldContext, FName TargetParam, FName SourceParam);

    UFUNCTION(BlueprintCallable, BlueprintCosmetic, Meta = (DisplayName = "MPC_Global_MapParameter", WorldContext = "WorldContext"))
    static void UnmapParameter(UObject* WorldContext, FName TargetParam);

    /**
    * Return all parameters mapped to this source parameter.
    */
    UFUNCTION(BlueprintPure, BlueprintCosmetic, Meta = (DisplayName = "MPC_Global_GetMappedParameters", WorldContext = "WorldContext"))
    static void GetMappedParameters(UObject* WorldContext, FName SourceParam, TSet<FName>& OutParams);

    /**
    * Return the source parameter this parameter is mapped to, or NAME_None if not mapped.
    */
    UFUNCTION(BlueprintPure, BlueprintCosmetic, Meta = (DisplayName = "MPC_Global_GetMapping", WorldContext = "WorldContext"))
    static FName GetMapping(UObject* WorldContext, FName TargetParam);

protected:

    /**
    * Mapping of parameters to other parameters.
    * Keys are TARGET parameters (followers). A target can only be mapped to one source.
    * Values are SOURCE parameters (providers). A source can supply multiple targets.
    */
    UPROPERTY()
    TMap<FName, FName> Mappings;

};