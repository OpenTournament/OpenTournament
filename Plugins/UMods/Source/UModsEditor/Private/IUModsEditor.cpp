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

#include "IUModsEditor.h"
#include "Modules/ModuleManager.h"
#include "CommandContextUMods.h"
#include "Slate/DockTabUMods.h"
#include "Brushes/SlateImageBrush.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "HAL/PlatformFilemanager.h"
#include "Interfaces/IPluginManager.h"
#include "LevelEditor.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

// ---------------------------------------------------------------------------
//		Module Implementation
// ---------------------------------------------------------------------------

IMPLEMENT_MODULE(IUModsEditor, UModsEditor);

// ---------------------------------------------------------------------------
//		IUModsEditor - IModuleInterface
// ---------------------------------------------------------------------------

void IUModsEditor::StartupModule()
{
	// Crash prevention.
	if (!FSlateApplication::IsInitialized())
	{
		return;
	}

	// Register a new slate style.
	SlateStyleSetPtr = MakeShareable(new FSlateStyleSet("UModsEditorStyle"));
	SlateStyleSetPtr->SetContentRoot(IPluginManager::Get().FindPlugin(TEXT("UMods"))->GetBaseDir() / TEXT("Resources"));
	SlateStyleSetPtr->Set("UModsEditor.OpenMenu", new FSlateImageBrush(SlateStyleSetPtr->RootToContentDir(TEXT("Icon"), TEXT(".png")), FVector2D(40.0f, 40.0f)));
	FSlateStyleRegistry::RegisterSlateStyle(*SlateStyleSetPtr);
	FSlateApplication::Get().GetRenderer()->ReloadTextureResources();

	// Register the button's command.
	CommandContextUMods::Register();
	UICommandListPtr = MakeShareable(new FUICommandList);
	UICommandListPtr->MapAction(CommandContextUMods::Get().OpenMenu, FExecuteAction::CreateRaw(this, &IUModsEditor::ModsButtonReleased), FCanExecuteAction());

	// Create the button.
	TSharedPtr<class FExtender> ExtenderPtr = MakeShareable(new FExtender);
	ExtenderPtr->AddToolBarExtension("Game", EExtensionHook::After, UICommandListPtr, FToolBarExtensionDelegate::CreateRaw(this, &IUModsEditor::ModsButtonCreated));
	FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor").GetToolBarExtensibilityManager()->AddExtender(ExtenderPtr);

	// Create the dock tab.
	FTabSpawnerEntry& TabSpawnerEntry = FGlobalTabmanager::Get()->RegisterNomadTabSpawner("UMods", FOnSpawnTab::CreateRaw(this, &IUModsEditor::ModsDockTabCreated));
	FTabManager::RegisterDefaultTabWindowSize(FName("UMods"), FVector2D(1160.0f, 720.0f));
	TabSpawnerEntry.SetDisplayName(INVTEXT("Mods"));
	TabSpawnerEntry.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void IUModsEditor::ShutdownModule()
{
	// Crash prevention.
	if (!FSlateApplication::IsInitialized())
	{
		return;
	}

	// Unregister the button's command.
	CommandContextUMods::Unregister();
	UICommandListPtr.Reset();

	// Unregister the slate style.
	FSlateStyleRegistry::UnRegisterSlateStyle(*SlateStyleSetPtr);
	SlateStyleSetPtr.Reset();

	// Destroy the dock tab.
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner("UMods");
}

// ---------------------------------------------------------------------------
//		IUModsEditor
// ---------------------------------------------------------------------------

void IUModsEditor::ModsButtonCreated(FToolBarBuilder& ToolBarBuilder)
{
	ToolBarBuilder.AddSeparator();
	ToolBarBuilder.AddToolBarButton(CommandContextUMods::Get().OpenMenu);
}

void IUModsEditor::ModsButtonReleased()
{
	FGlobalTabmanager::Get()->InvokeTab(FName("UMods"));
}

TSharedRef<SDockTab> IUModsEditor::ModsDockTabCreated(const FSpawnTabArgs& SpawnTabArgs)
{
	TSharedRef<SDockTabUMods> DockTab = SNew(SDockTabUMods).TabRole(ETabRole::NomadTab);
	DockTab->Set();
	return DockTab;
}

// ---------------------------------------------------------------------------
