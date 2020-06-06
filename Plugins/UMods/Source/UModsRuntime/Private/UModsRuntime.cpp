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

#include "UModsRuntime.h"

#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/App.h"
#include "Misc/CoreDelegates.h"
#include "UObject/Package.h"
#include "UObject/UObjectIterator.h"
#include "IPlatformFilePak.h"

#include "HAL/PlatformFilemanager.h"


// ---------------------------------------------------------------------------
//		Log
// ---------------------------------------------------------------------------

DEFINE_LOG_CATEGORY(LogUMods);

// ---------------------------------------------------------------------------
//		Accessor
// ---------------------------------------------------------------------------

UUMods* UUMods::GetUMods()
{
	return UUMods::StaticClass()->GetDefaultObject<UUMods>();
}

// ---------------------------------------------------------------------------
//		Utilities
// ---------------------------------------------------------------------------

void UUMods::Scan(TArray<FString>& Loaded, TArray<FString>& Loading, TArray<FString>& Unloaded, TArray<FString>& Unloading)
{
	Loaded.Empty();
	Unloaded.Empty();
	for (TSharedRef<IPlugin> Plugin : IPluginManager::Get().GetDiscoveredPlugins())
	{
		if (Plugin->GetType() == EPluginType::Mod && !Loading.Contains(Plugin->GetName()) && !Unloading.Contains(Plugin->GetName()))
		{
			if (Plugin->IsEnabled())
			{
				Loaded.Add(Plugin->GetName());
			}
			else
			{
				Unloaded.Add(Plugin->GetName());
			}
		}
	}
	Loading = ModsLoading;
	Unloading = ModsUnloading;
}

void UUMods::LoadMod(FString Mod)
{
	for (TSharedRef<IPlugin> Plugin : IPluginManager::Get().GetDiscoveredPlugins())
	{
		if (Plugin->GetName() == Mod)
		{
			if (Plugin->GetType() == EPluginType::Mod && !Plugin->IsEnabled() && !ModsLoading.Contains(Plugin->GetName()) && !ModsUnloading.Contains(Plugin->GetName()))
			{
				// TODO - Refactor the whole "if" to be asynchronous.

				//ModsLoading.Add(Plugin->GetName());
				FMod ModTemp;
				ModTemp.Name = Plugin->GetName();
				for (FModuleDescriptor ModuleDescriptor : Plugin->GetDescriptor().Modules)
				{
					FModule Module;
					Module.Name = ModuleDescriptor.Name.ToString();
					ModTemp.Modules.Add(Module);
				}
				IPluginManager::Get().MountNewlyCreatedPlugin(Plugin->GetName());
				if (Plugin->CanContainContent())
				{
					IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
					TArray<FString> Files;
#if WITH_EDITOR
					PlatformFile.FindFilesRecursively(Files, *Plugin->GetContentDir(), nullptr);
#else
					FString Pak = Plugin->GetContentDir() / FString(Plugin->GetName() + TEXT("-") + FPlatformProperties::PlatformName() + TEXT(".pak"));

					// Temporary. This is the expected format when mods are packaged with UFE.
					if (!PlatformFile.FileExists(*Pak))
					{
						Pak = Plugin->GetContentDir() / FString(Plugin->GetName() + FApp::GetProjectName() + TEXT("-") + FPlatformProperties::PlatformName() + TEXT(".pak"));
					}

					if (PlatformFile.FileExists(*Pak) && FCoreDelegates::OnMountPak.IsBound() && FCoreDelegates::OnMountPak.Execute(Pak, 0, nullptr))
					{
						FPakFile* PakFile = new FPakFile(&PlatformFile, *Pak, false);
						PakFile->FindFilesAtPath(Files, *PakFile->GetMountPoint(), true, false, true);
					}
#endif
					for (FString& File : Files)
					{
						FString FilePathIrrelevant, FilePathRelevant;
						File.Split(TEXT("/") + Plugin->GetName() + TEXT("/Content/"), &FilePathIrrelevant, &FilePathRelevant);
						if (FilePathRelevant.EndsWith(TEXT(".umap")))
						{
							ModTemp.Content.Levels.Add(TEXT("/") + Plugin->GetName() + TEXT("/") + FilePathRelevant.LeftChop(5));
						}
						else if (FilePathRelevant.EndsWith(TEXT(".uasset")) && !FilePathRelevant.Contains(TEXT("_BuiltData")))
						{
							// Change to LoadPackageAsync()
							LoadPackage(nullptr, *FString(TEXT("/") + Plugin->GetName() + TEXT("/") + FilePathRelevant.LeftChop(7)), LOAD_None);
						}
					}
				}

				for (TObjectIterator<UClass> Class; Class; ++Class)
				{
					if (Class->GetPathName().StartsWith(TEXT("/") + ModTemp.Name + TEXT("/")))
					{
						FString A, B;
						TArray<FString> Strings;
						Class->GetPathName().ParseIntoArray(Strings, TEXT("/"));
						Strings.Last().Split(TEXT("."), &A, &B);
						if (A == B.LeftChop(2))
						{
							ModTemp.Content.Classes.Add(LoadObject<UClass>(nullptr, *Class->GetPathName()));
						}
					}
					else
					{
						for (FModule& Module : ModTemp.Modules)
						{
							if (Class->GetPathName().StartsWith(TEXT("/Script/") + Module.Name + TEXT(".")))
							{
								Module.Classes.Add(LoadObject<UClass>(nullptr, *Class->GetPathName()));
								break;
							}
						}
					}
				}

				UE_LOG(LogUMods, Warning, TEXT("Mod Name: %s"), *ModTemp.Name);
				for (FModule& Module : ModTemp.Modules)
				{
					UE_LOG(LogUMods, Warning, TEXT("Module Name: %s"), *Module.Name);
					for (UClass*& Class : Module.Classes)
					{
						UE_LOG(LogUMods, Warning, TEXT("Module Class: %s"), *Class->GetFName().ToString());
					}
				}
				for (FString& Level : ModTemp.Content.Levels)
				{
					UE_LOG(LogUMods, Warning, TEXT("Content Level: %s"), *Level);
				}
				for (UClass*& Class : ModTemp.Content.Classes)
				{
					UE_LOG(LogUMods, Warning, TEXT("Content Class: %s"), *Class->GetFName().ToString());
				}
				UE_LOG(LogUMods, Warning, TEXT("--------------------------------------------------"));

				//ModsLoading.Remove(Plugin->GetName());
				OnModLoaded.Broadcast(ModTemp.Name, ModTemp.Modules, ModTemp.Content);
			}
			else
			{
				FString Error;
				if (Plugin->GetType() != EPluginType::Mod)
				{
					Error = TEXT("The mod does not reside in the <Mods> folder.");
				}
				else if (Plugin->IsEnabled())
				{
					Error = TEXT("The mod is already loaded.");
				}
				else if (ModsLoading.Contains(Plugin->GetName()))
				{
					Error = TEXT("The mod is being loaded.");
				}
				else if (ModsUnloading.Contains(Plugin->GetName()))
				{
					Error = TEXT("The mod is being unloaded.");
				}
				UE_LOG(LogUMods, Warning, TEXT("LoadMod: %s"), *Error);
				return;
			}
			break;
		}
	}
	UE_LOG(LogUMods, Warning, TEXT("LoadMod: Mod not found."));
}

void UUMods::UnloadMod(FString Mod)
{

}

// ---------------------------------------------------------------------------
//		Internal
// ---------------------------------------------------------------------------

UUMods::UUMods()
{
#if !WITH_EDITOR
	IPluginManager::Get().AddPluginSearchPath(FPaths::ProjectPluginsDir());
	IPluginManager::Get().AddPluginSearchPath(FPaths::ProjectModsDir()); // Maybe
#endif
}

UUMods::~UUMods()
{

}

// ---------------------------------------------------------------------------
