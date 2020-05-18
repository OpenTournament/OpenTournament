// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "UR_FunctionLibrary.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations

class APlayerState;
class AUR_GameModeBase;
class AUR_Character;
class UFXSystemComponent;
class UAnimInstance;
class UAnimMontage;
class UActorComponent;

/////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * 
 */
UCLASS()
class OPENTOURNAMENT_API UUR_FunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    /**
     * Utility for retrieving GameMode CDO
     */
    UFUNCTION(BlueprintPure, Category = "Utility")
    static AUR_GameModeBase* GetGameModeDefaultObject(const UObject* WorldContextObject);


    /**
     * Utility to retrieve the String value of a given Enum
     */
    template<typename TEnum>
    static FORCEINLINE FString GetEnumValueAsString(const FString& Name, TEnum Value)
    {
        const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, *Name, true);
        if (!EnumPtr)
        {
            return FString("Invalid");
        }
        return EnumPtr->GetNameByValue(static_cast<int64>(Value)).ToString();
    }


    /**
    * Resolve the text color a player should be written with.
    * Used for chat, death messages... [insert more]
    * Use to keep consistency around interfaces.
    * Especially considering that later on, players might have :
    * - Customizable team colors
    * - Customizable self color in non team games
    */
    UFUNCTION(BlueprintPure)
    static FColor GetPlayerDisplayTextColor(const APlayerState* PS);


    /**
    * Text color for spectators.
    * Use to keep consistency around interfaces.
    */
    UFUNCTION(BlueprintPure)
    static FColor GetSpectatorDisplayTextColor() { return FColor(255, 200, 0, 255); }


    /**
    * Returns a modified version of the string, where rich-text special characters are escaped.
    * If printed as-is, characters like < will be encoded like &lt;
    * If used in a RichTextBlock, they will be displayed properly.
    */
    UFUNCTION(BlueprintPure)
    static FString EscapeForRichText(const FString& InText)
    {
        // see RichTextMarkupProcessing.cpp
        return InText.Replace(TEXT("&"), TEXT("&amp;"))
            .Replace(TEXT("\""), TEXT("&quot;"))
            .Replace(TEXT("<"), TEXT("&lt;"))
            .Replace(TEXT(">"), TEXT("&gt;"));
    }


    /**
    * Returns a modified version of the string, where encoded rich-text special characters are restored.
    */
    UFUNCTION(BlueprintPure)
    static FString UnescapeRichText(const FString& InText)
    {
        return InText.Replace(TEXT("&gt;"), TEXT(">"))
            .Replace(TEXT("&lt;"), TEXT("<"))
            .Replace(TEXT("&quot;"), TEXT("\""))
            .Replace(TEXT("&amp;"), TEXT("&"));
    }


    /**
    * Returns a modified version of the string, where rich-text decorators are visible but undetectable.
    */
    UFUNCTION(BlueprintPure)
    static FString BreakRichTextDecorators(const FString& InText)
    {
        return InText.Replace(TEXT("/>"), TEXT("/\u200B>"));	// zero width space
    }


    /**
    * Returns a modified version of the string without any rich-text decorators.
    * Only markers are removed. The content within decorators is left untouched.
    */
    UFUNCTION(BlueprintPure)
    static FString StripRichTextDecorators(const FString& InText);


    /**
    * Returns true if given key is mapped to given action.
    * Does not consider modifiers.
    *
    * This is a helper for doing special UMG navigation via OnKeyDown,
    * using realtime binds and not hardcoded.
    */
    UFUNCTION(BlueprintPure)
    static bool IsKeyMappedToAction(const FKey& Key, const FName ActionName);


    /**
    * Returns true if given key is mapped to given axis.
    * Use direction 0 for any.
    *
    * This is a helper for doing special UMG navigation via OnKeyDown,
    * using realtime binds and not hardcoded.
    */
    UFUNCTION(BlueprintPure)
    static bool IsKeyMappedToAxis(const FKey& Key, const FName AxisName, const float Direction = 1.f);


    /**
    * Get local player controller as UR_PlayerController.
    *
    * Flagged as cosmetic-only to put emphasis on client-side usage.
    * Should be avoided on dedicated server code as the result would be random.
    */
    UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "Game", Meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
    static class AUR_PlayerController* GetLocalPlayerController(const UObject* WorldContextObject);


    /**
    * Returns true if actor is currently viewed by local player controller.
    */
    UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "Game")
    static bool IsLocallyViewed(const AActor* Other);

    /**
    * Returns true if we are currently viewing this character in first person.
    */
    UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "Game")
    static bool IsViewingFirstPerson(const AUR_Character* Other);

    /**
    * Returns true if actor is locally controlled, ie. topmost owner is the local playercontroller.
    * Equivalent to calling Actor->HasLocalNetOwner(), but with BP support and gained uniformity.
    * If NetMode is NM_Client, this is a condition to being able to call server RPCs.
    */
    UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "Game")
    static bool IsLocallyControlled(const AActor* Other);

    /**
    * Returns true if component is locally controlled, ie. topmost owner is the local playercontroller.
    * If NetMode is NM_Client, this is a condition to being able to call server RPCs.
    */
    UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "Game")
    static bool IsComponentLocallyControlled(const UActorComponent* Other);


    /**
    * Get the Time as a String
    */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FunctionLibrary")
    static FString GetTimeString(const float TimeSeconds);


    /**
    * Random vector between 2 vectors.
    */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Math")
    static FVector RandomVectorInRange(const FVector& Vector1, const FVector& Vector2)
    {
        return FVector(FMath::RandRange(Vector1.X, Vector2.X), FMath::RandRange(Vector1.Y, Vector2.Y), FMath::RandRange(Vector1.Z, Vector2.Z));
    }


    /**
    * Spawn effect at location - niagara/particle independent.
    */
    UFUNCTION(BlueprintCallable, Category = "Effects")
    static UFXSystemComponent* SpawnEffectAtLocation(UWorld* World, UFXSystemAsset* Template, const FTransform& Transform, bool bAutoDestroy = true, bool bAutoActivate = true);

    /**
    * Spawn effect attached - niagara/particle independent.
    */
    UFUNCTION(BlueprintCallable, Category = "Effects")
    static UFXSystemComponent* SpawnEffectAttached(UFXSystemAsset* Template, const FTransform& Transform, USceneComponent* AttachToComponent, FName AttachPointName = NAME_None, EAttachLocation::Type LocationType = EAttachLocation::KeepRelativeOffset, bool bAutoDestroy = true, bool bAutoActivate = true);


    /**
    * C++ utility to cast (checked) a TScriptInterface<IBase> to a TScriptInterface<IDerived>
    */
    template<class IBase, class IDerived> static void CastScriptInterface(const TScriptInterface<IBase>& Base, TScriptInterface<IDerived>& OutDerived)
    {
        UObject* Object = Base.GetObject();
        IDerived* Casted = Cast<IDerived>(Object);
        if (Casted)
        {
            OutDerived.SetObject(Object);
            OutDerived.SetInterface(Casted);
        }
        else
        {
            OutDerived.SetObject(NULL);
            //OutDerived.SetInterface(NULL); //unnecessary
        }
    }


    /**
    * Get the current active montage in specified slot.
    */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Animation", Meta = (NotBlueprintThreadSafe))
    static UAnimMontage* GetCurrentActiveMontageInSlot(UAnimInstance* AnimInstance, FName SlotName, bool& bIsValid, float& Weight);

};
