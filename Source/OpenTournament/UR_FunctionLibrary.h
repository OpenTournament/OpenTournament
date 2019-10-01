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


	/**
	* Resolve the text color a player should be written with.
	* Used for chat, death messages... [insert more]
	* Use to keep consistency around interfaces.
	* Especially considering that later on, players might have :
	* - customizeable team colors
	* - customizeable self color in non team games
	*/
	UFUNCTION(BlueprintPure)
	static FColor GetPlayerDisplayTextColor(APlayerState* PS);


	/**
	* Text color for spectators.
	* Use to keep consistency around interfaces.
	*/
	UFUNCTION(BlueprintPure)
	static FColor GetSpectatorDisplayTextColor() { return FColorList::Gold; }


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
};
