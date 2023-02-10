// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"

#include "UR_FunctionLibrary.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations

class APlayerState;
class AGameModeBase;
class AUR_GameModeBase;
class AUR_Character;
class UFXSystemComponent;
class UAnimInstance;
class UAnimMontage;
class UActorComponent;
class AUR_PlayerController;
class UWidget;
class UMeshComponent;
class UMaterialInterface;
class UInterface;
class AUR_Weapon;
class UFXSystemAsset;

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
    static AGameModeBase* GetGameModeBaseDefaultObject(const UObject* WorldContextObject);

    /**
     * Utility for retrieving GameMode CDO
     */
    template<class T>
    static FORCEINLINE T* GetGameModeDefaultObject(const UObject* WorldContextObject)
    {
        return Cast<T>(GetGameModeBaseDefaultObject(WorldContextObject));
    }

    /**
     * Utility for retrieving GameMode CDO
     */
    UFUNCTION(BlueprintPure, Category = "Utility", Meta = (WorldContext = "WorldContextObject"))
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
    static FColor GetPlayerDisplayTextColor(APlayerState* PS);


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
    static AUR_PlayerController* GetLocalPlayerController(const UObject* WorldContextObject);

    static APlayerController* GetLocalPC(const UObject* WorldContextObject);
    template<typename T> static T* GetLocalPC(const UObject* WorldContextObject)
    {
        return Cast<T>(GetLocalPC(WorldContextObject));
    }

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
    UFUNCTION(BlueprintCallable, Category = "Effects", Meta = (WorldContext = "WorldContextObject"))
    static UFXSystemComponent* SpawnEffectAtLocation(const UObject* WorldContextObject, UFXSystemAsset* Template, const FTransform& Transform, bool bAutoDestroy = true, bool bAutoActivate = true);

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


    UFUNCTION(BlueprintPure, Category = "Text")
    static void ParseFloatTextInput(FText Text, bool& bIsNumeric, float& OutValue);

    UFUNCTION(BlueprintPure, Category = "Game")
    static bool IsOnlySpectator(APlayerState* PS);

    /** Returns true if A is between B and C (B <= A <= C) */
    UFUNCTION(BlueprintPure, meta = (DisplayName = "integer between", CompactNodeTitle = "<=>"), Category = "Math|Integer")
    static bool Between_IntInt(int32 A, int32 B, int32 C)
    {
        return A >= B && A <= C;
    }

    /** Returns true if A is between B and C (B <= A <= C) */
    UFUNCTION(BlueprintPure, meta = (DisplayName = "float between", CompactNodeTitle = "<=>"), Category = "Math|Float")
    static bool Between_FloatFloat(float A, float B, float C)
    {
        return A >= B && A <= C;
    }

    UFUNCTION(BlueprintPure, Category = "Game Options", meta = (BlueprintThreadSafe))
    static float GetFloatOption(const FString& Options, const FString& Key, float DefaultValue);

    UFUNCTION(BlueprintCallable, Category = "UMG", Meta = (DeterminesOutputType = "WidgetClass", DynamicOutputParam = "OutWidgets"))
    static bool FindChildrenWidgetsByClass(UWidget* Target, TSubclassOf<UWidget> WidgetClass, TArray<UWidget*>& OutWidgets, bool bRecursive = true);

    /**
    * Remove all override materials from a mesh component, restoring it to its defaults.
    */
    UFUNCTION(BlueprintCallable, Category = "Mesh|Material")
    static void ClearOverrideMaterials(UMeshComponent* MeshComp);

    /**
    * Override all materials of a mesh component with a single material.
    */
    UFUNCTION(BlueprintCallable, Category = "Mesh|Material")
    static void OverrideAllMaterials(UMeshComponent* MeshComp, UMaterialInterface* Material);

    UFUNCTION(BlueprintCallable)
    static void GetAllWeaponClasses(TSubclassOf<AUR_Weapon> InClassFilter, TArray<TSubclassOf<AUR_Weapon>>& OutWeaponClasses);

    UFUNCTION(BlueprintPure, CustomThunk, Category = "Utilities|Array", Meta = (ArrayParm = "Source", ArrayTypeDependentParams = "Result"))
    static void Array_Slice(const TArray<int32>& Source, TArray<int32>& Result, int32 Start, int32 End = -1);
    static void GenericArray_Slice(void* SourceArray, const FArrayProperty* SourceArrayProp, void* ResultArray, const FArrayProperty* ResultArrayProp, int32 Start, int32 End);
    DECLARE_FUNCTION(execArray_Slice)
    {
        // Retrieve the target array
        Stack.MostRecentProperty = nullptr;
        Stack.StepCompiledIn<FArrayProperty>(NULL);
        void* SourceArrayAddr = Stack.MostRecentPropertyAddress;
        FArrayProperty* SourceArrayProp = CastField<FArrayProperty>(Stack.MostRecentProperty);
        if (!SourceArrayProp)
        {
            Stack.bArrayContextFailed = true;
            return;
        }
        // Retrieve the result array
        Stack.MostRecentProperty = nullptr;
        Stack.StepCompiledIn<FArrayProperty>(NULL);
        void* ResultArrayAddr = Stack.MostRecentPropertyAddress;
        FArrayProperty* ResultArrayProp = CastField<FArrayProperty>(Stack.MostRecentProperty);
        if (!ResultArrayProp)
        {
            Stack.bArrayContextFailed = true;
            return;
        }

        P_GET_PROPERTY(FIntProperty, Start);
        P_GET_PROPERTY(FIntProperty, End);
        P_FINISH;
        P_NATIVE_BEGIN;
        GenericArray_Slice(SourceArrayAddr, SourceArrayProp, ResultArrayAddr, ResultArrayProp, Start, End);
        P_NATIVE_END;
    }

    // c++ version
    template<typename T> static TArray<T> ArraySlice(const TArray<T>& InArray, int32 Start, int32 End = -1);

    UFUNCTION(BlueprintPure, Category = "Utilities")
    static bool ClassImplementsInterface(UClass* TestClass, TSubclassOf<UInterface> Interface);

    UFUNCTION(BlueprintPure, Category = "Utilities|Text")
    static FText JoinTextArray(const TArray<FText>& SourceArray, const FString& Separator = FString(TEXT(" ")))
    {
        return FText::Join(FText::FromString(Separator), SourceArray);
    }

    /**
    * Find and return all tags in TagContainer matching TagToMatch.*
    * Example container: {
    *   Reward
    *   Reward.MultiKill
    *   Reward.MultiKill.Double
    * }
    * FindChildTags(Reward)           --> { Reward.MultiKill, Reward.MultiKill.Double }
    * FindChildTags(Reward.MultiKill) --> { Reward.MultiKill.Double }
    */
    UFUNCTION(BlueprintPure, Category = "GameplayTags")
    static FGameplayTagContainer FindChildTags(const FGameplayTagContainer& TagContainer, FGameplayTag TagToMatch);

    /**
    * Find and return any tag in TagContainer matching TagToMatch.*
    * If several are found, only one is returned. Order is not guaranteed.
    * If none are found, an empty GameplayTag is returned. Check result with IsValid.
    * Example container: {
    *   Reward
    *   Reward.MultiKill
    *   Reward.MultiKill.Double
    * }
    * FindChildTags(Reward.MultiKill) --> Reward.MultiKill.Double
    */
    UFUNCTION(BlueprintPure, Category = "GameplayTags")
    static FGameplayTag FindAnyChildTag(const FGameplayTagContainer& TagContainer, FGameplayTag TagToMatch);

    UFUNCTION(BlueprintPure, Category = "Math|Vector")
    static FORCEINLINE FVector ClampVector(const FVector& V, const FVector& Min, const FVector& Max)
    {
        return FVector(
            FMath::Clamp(V.X, Min.X, Max.X),
            FMath::Clamp(V.Y, Min.Y, Max.Y),
            FMath::Clamp(V.Z, Min.Z, Max.Z)
        );
    }

    UFUNCTION(BlueprintPure, Category = "Math|Vector")
    static FORCEINLINE FVector2D ClampVector2D(const FVector2D& V, const FVector2D& Min, const FVector2D& Max)
    {
        return FVector2D(
            FMath::Clamp(V.X, Min.X, Max.X),
            FMath::Clamp(V.Y, Min.Y, Max.Y)
        );
    }

    /**
    * Force refresh bone transforms on a skeletal mesh.
    * Useful when you need to read a bone/socket transform from a mesh that may not have been rendered recently.
    * Only makes sense when VisibilityBasedAnimTickOption == AlwaysTickPose (Always Tick, but Refresh BoneTransforms only when rendered)
    */
    UFUNCTION(BlueprintCallable, Category = "Game")
    static void RefreshBoneTransforms(USkeletalMeshComponent* SkelMesh);

    /**
    * Walks up the chain of parents (including self) to call RefreshBoneTransforms on SkeletalMeshes.
    * Will not work properly if a parent has VisibilityBasedAnimTickOption below AlwaysTickPose (ie. not ticking anims at all)
    */
    UFUNCTION(BlueprintCallable, Category = "Game")
    static void RefreshComponentTransforms(USceneComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Game")
    static void PropagateOwnerNoSee(USceneComponent* Component, bool bOwnerNoSee);

};
