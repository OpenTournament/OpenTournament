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

#include "Slate/DockTabUMods.h"

#include "HAL/PlatformFilemanager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SEditableText.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Text/SMultiLineEditableText.h"
#include "Widgets/Text/STextBlock.h"
#include "Slate/CompoundWidgetUModsCreate.h"
#include "Slate/CompoundWidgetUModsLog.h"
#include "Slate/CompoundWidgetUModsPackage.h"
#include "Slate/Styles.h"

void SDockTabUMods::OnReleaseSelected(ECheckBoxState State)
{
	if (State == ECheckBoxState::Unchecked)
	{
		return;
	}
	bool bSelectionFound = false;
	for (int i = 0; i < ReleasesEntries.Num(); i++)
	{
		if (ReleasesEntries[i]->IsChecked() && ReleasesNames[i] != Release && !bSelectionFound)
		{
			Release = ReleasesNames[i];
			bSelectionFound = true;
			UE_LOG(LogTemp, Warning, TEXT("CHECKED: %s"), *Release);
		}
		else
		{
			ReleasesEntries[i]->SetIsChecked(ECheckBoxState::Unchecked);
		}
	}
}

void SDockTabUMods::Set()
{
	TArray<FString> Files;
	FPlatformFileManager::Get().GetPlatformFile().FindFilesRecursively(Files, *FString(FPaths::ProjectDir() / TEXT("Releases")), TEXT("bin"));
	TArray<FString> Releases;
	FString A, B, C, D;
	for (FString& File : Files)
	{
		File.Split(*FString(FPaths::ProjectDir() / TEXT("Releases")), &A, &B);
		B.RightChop(1).Split(TEXT("/"), &C, &D);
		Releases.AddUnique(C);
	}

	TSharedRef<SBorder> Border = SNew(SBorder);
	SetContent(Border);

	if (Releases.Num() > 0)
	{
		TSharedRef<SSplitter> SplitterA = SNew(SSplitter);
		{
			SSplitter::FSlot& SplitterSlotA1 = SplitterA->AddSlot();
			{
				TSharedRef<SSplitter> SplitterB = SNew(SSplitter);
				{
					SSplitter::FSlot& SplitterSlotB1 = SplitterB->AddSlot();
					{
						TSharedRef<SCompoundWidgetUModsCreate> Widget = SNew(SCompoundWidgetUModsCreate);
						SplitterSlotB1.Value(0.38f);
						SplitterSlotB1.AttachWidget(Widget);
					}
					SSplitter::FSlot& SplitterSlotB2 = SplitterB->AddSlot();
					{
						TSharedRef<SCompoundWidgetUModsPackage> Widget = SNew(SCompoundWidgetUModsPackage);
						SplitterSlotB2.Value(0.62f);
						SplitterSlotB2.AttachWidget(Widget);
					}
					SplitterB->SetOrientation(Orient_Vertical);
				}
				SplitterSlotA1.Value(0.38f);
				SplitterSlotA1.AttachWidget(SplitterB);
			}
			SSplitter::FSlot& SplitterSlotA2 = SplitterA->AddSlot();
			{
				SplitterSlotA2.Value(0.62f);
				SplitterSlotA2.AttachWidget(SNew(SCompoundWidgetUModsLog));
			}
		}
		Border->SetBorderImage(FStyles::GetBackgroundBlack());
		Border->SetPadding(FStyles::GetPaddingNone());
		Border->SetContent(SplitterA);
	}
	else
	{
		TSharedRef<STextBlock> Text = SNew(STextBlock);
		Text->SetText(INVTEXT("No releases found. The UMods plugin assumes the developers have created at least one release version of the project."));
		Text->SetJustification(ETextJustify::Type::Center);
		Border->SetHAlign(HAlign_Center);
		Border->SetVAlign(VAlign_Center);
		Border->SetContent(Text);
	}

}