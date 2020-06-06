// ---------------------------------------------------------------------------------------------------- //
//                                                                                                      //
// Copyright (C) Vlad Serbanescu 2019 - All Rights Reserved. Unauthorized copying of this file, via any //
// medium is strictly prohibited. Proprietary and confidential.                                         //
//                                                                                                      //
// By downloading and/or using the UMods plugin (the “Product”) or any part of it, you indicate         //
// your acceptance to respect the license provided by Vlad Serbanescu (the “Licensor”) below.           //
//                                                                                                      //
// The Licensor grants you a non-exclusive, non-transferable, non-sublicensable license for a single    //
// user to use, reproduce, display, perform, and modify the Product for Unreal Engine 4 (R) for any     //
// lawful purpose (the "License").                                                                      //
//                                                                                                      //
// The License becomes effective on the date you buy the Product for Unreal Engine 4 (R). The License   //
// does not grant you any title or ownership in the licensed technology. You may distribute the Product //
// for Unreal Engine 4 (R) incorporated in object code format only as an inseparable part of a product  //
// to end users. The product may not contain any plugin content in uncooked source format.              //
//                                                                                                      //
// Unreal (R) is a trademark or registered trademark of Epic Games Inc. in the United States of America //
// and elsewhere. Unreal (R) Engine, Copyright 1998 - 2019, Epic Games, Inc. All rights reserved.       //
//                                                                                                      //
// By executing, reading, editing, copying or keeping files from this file, you agree to the following  //
// terms, in addition to Epic Games Marketplace EULA:                                                   //
//                                                                                                      //
// - You have read and agree to terms provided by Epic Games at: https://publish.unrealengine.com/faq   //
// - You agree the Licensor reserves all rights to this file, granted by law.                           //
// - You agree the Licensor will not provide any part of this file outside the Marketplace environment. //
// - You agree the Licensor provided support channels which are under his sole discretion.              //
//                                                                                                      //
// ---------------------------------------------------------------------------------------------------- //

#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"
#include "UModsRuntime.generated.h"

// ---------------------------------------------------------------------------
//		Log
// ---------------------------------------------------------------------------

DECLARE_LOG_CATEGORY_EXTERN(LogUMods, Log, All);

// ---------------------------------------------------------------------------
//		Structs
// ---------------------------------------------------------------------------

/**
* Struct containing information about a mod's module.
*/
USTRUCT(BlueprintType)
struct UMODSRUNTIME_API FModule
{

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "UMods")
		FString Name;

	UPROPERTY(BlueprintReadWrite, Category = "UMods")
		TArray<UClass*> Classes;

};

/**
* Struct containing information about a mod's content.
*/
USTRUCT(BlueprintType)
struct UMODSRUNTIME_API FContent
{

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "UMods")
		TArray<FString> Levels;

	UPROPERTY(BlueprintReadWrite, Category = "UMods")
		TArray<UClass*> Classes;

};

/**
* Struct containing information about a mod.
*/
USTRUCT(BlueprintType)
struct UMODSRUNTIME_API FMod
{

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "UMods")
		FString Name;

	UPROPERTY(BlueprintReadWrite, Category = "UMods")
		TArray<FModule> Modules;

	UPROPERTY(BlueprintReadWrite, Category = "UMods")
		FContent Content;

};

// ---------------------------------------------------------------------------
//		Delegates
// ---------------------------------------------------------------------------

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FModLoadedDelegate, FString, Name, const TArray<FModule>&, Modules, FContent, Content);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FModUnloadedDelegate, FString, Name);

// ---------------------------------------------------------------------------
//		Classes
// ---------------------------------------------------------------------------

/**
* Class containing functions to interact with mods.
*/
UCLASS(Blueprintable)
class UMODSRUNTIME_API UUMods : public UObject
{

	GENERATED_BODY()

	// ---------------------------------------------------------------------------
	//		Accessor
	// ---------------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "UMods")
		static UUMods* GetUMods();

	// ---------------------------------------------------------------------------
	//		Utilities
	// ---------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "UMods")
		void Scan(TArray<FString>& Loaded, TArray<FString>& Loading, TArray<FString>& Unloaded, TArray<FString>& Unloading);

	UFUNCTION(BlueprintCallable, Category = "UMods")
		void LoadMod(FString Mod);

	UFUNCTION(BlueprintCallable, Category = "UMods")
		void UnloadMod(FString Mod);

	// ---------------------------------------------------------------------------
	//		Callbacks
	// ---------------------------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "UMods")
		FModLoadedDelegate OnModLoaded;

	UPROPERTY(BlueprintAssignable, Category = "UMods")
		FModUnloadedDelegate OnModUnloaded;

	// ---------------------------------------------------------------------------

private:

	// ---------------------------------------------------------------------------
	//		Internal
	// ---------------------------------------------------------------------------

	UUMods();

	~UUMods();

	TArray<FString> ModsLoading;

	TArray<FString> ModsUnloading;

	// ---------------------------------------------------------------------------

};

// ---------------------------------------------------------------------------
