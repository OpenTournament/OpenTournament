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

#include "Slate/CompoundWidgetUModsPackage.h"

#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/PlatformFilemanager.h"
#include "Slate/ButtonUMods.h"
#include "Slate/CheckBoxUMods.h"
#include "Slate/Styles.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void SCompoundWidgetUModsPackage::Construct(const FArguments& InArgs)
{
	TSharedRef<SBorder> Outline = SNew(SBorder);
	{
		TSharedRef<SVerticalBox> VerticalBox = SNew(SVerticalBox);
		{
			SVerticalBox::FSlot& VerticalBoxSlotA = VerticalBox->AddSlot();
			{
				TSharedRef<SBox> Box = SNew(SBox);
				{
					TSharedRef<SBorder> Border = SNew(SBorder);
					{
						TSharedRef<STextBlock> Text = SNew(STextBlock);
						Text->SetText(INVTEXT("Mods"));
						Text->SetJustification(ETextJustify::Center);
						Border->SetVAlign(VAlign_Center);
						Border->SetBorderImage(FStyles::GetBorderGreenBottom());
						Border->SetContent(Text);
					}
					Box->SetPadding(FStyles::GetBlockPadding());
					Box->SetHeightOverride(FStyles::GetBlockHeight());
					Box->SetContent(Border);
				}
				VerticalBoxSlotA.AutoHeight();
				VerticalBoxSlotA.AttachWidget(Box);
			}
			SVerticalBox::FSlot& VerticalBoxSlotB = VerticalBox->AddSlot();
			{
				TSharedRef<SScrollBox> ScrollBox = SNew(SScrollBox);
				{
					// Mods
				}
				VerticalBoxSlotB.Padding(FStyles::GetBlockPadding());
				VerticalBoxSlotB.VAlign(VAlign_Fill);
				VerticalBoxSlotB.AttachWidget(ScrollBox);
			}
			SVerticalBox::FSlot& VerticalBoxSlotC = VerticalBox->AddSlot();
			{
				TSharedRef<SBox> Box = SNew(SBox);
				{
					TSharedRef<SBorder> Border = SNew(SBorder);
					{
						TSharedRef<STextBlock> Text = SNew(STextBlock);
						Text->SetText(INVTEXT("Releases"));
						Text->SetJustification(ETextJustify::Center);
						Border->SetVAlign(VAlign_Center);
						Border->SetBorderImage(FStyles::GetBorderGreenHorizontal());
						Border->SetContent(Text);
					}
					Box->SetPadding(FStyles::GetBlockPadding());
					Box->SetHeightOverride(FStyles::GetBlockHeight());
					Box->SetContent(Border);
				}
				VerticalBoxSlotC.AutoHeight();
				VerticalBoxSlotC.AttachWidget(Box);
			}
			SVerticalBox::FSlot& VerticalBoxSlotD = VerticalBox->AddSlot();
			{
				TSharedRef<SScrollBox> ScrollBox = SNew(SScrollBox);
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
					for (FString& Release: Releases)
					{
						TSharedRef<SCheckBoxUMods> CheckBoxUMods = SNew(SCheckBoxUMods);
						CheckBoxUMods->TextBlock->SetText(FText::FromString(Release));
						CheckBoxUMods->OnSelected.BindRaw(this, &SCompoundWidgetUModsPackage::OnReleaseSelected);
						ScrollBox->AddSlot().AttachWidget(CheckBoxUMods);
					}
				}
				VerticalBoxSlotD.Padding(FStyles::GetBlockPadding());
				VerticalBoxSlotD.VAlign(VAlign_Fill);
				VerticalBoxSlotD.AttachWidget(ScrollBox);
			}
			SVerticalBox::FSlot& VerticalBoxSlotE = VerticalBox->AddSlot();
			{
				TSharedRef<SBox> Box = SNew(SBox);
				{
					TSharedRef<SBorder> Border = SNew(SBorder);
					{
						TSharedRef<SButtonUMods> Button = SNew(SButtonUMods);
						Button->TextBlock->SetText(INVTEXT("Package"));
						Button->OnSelected.BindRaw(this, &SCompoundWidgetUModsPackage::OnButtonReleased);;
						Border->SetBorderImage(FStyles::GetBorderGreenTop());
						Border->SetContent(Button);
					}
					Box->SetPadding(FStyles::GetBlockPadding());
					Box->SetHeightOverride(FStyles::GetBlockHeight());
					Box->SetContent(Border);
				}
				VerticalBoxSlotE.AutoHeight();
				VerticalBoxSlotE.AttachWidget(Box);
			}
		}
		Outline->SetBorderImage(FStyles::GetBorderGreenAll());
		Outline->SetContent(VerticalBox);
	}
	ChildSlot.AttachWidget(Outline);
}

void SCompoundWidgetUModsPackage::OnReleaseSelected(FString Text)
{
	UE_LOG(LogTemp, Warning, TEXT("Release: %s"), *Text);
}

void SCompoundWidgetUModsPackage::OnButtonReleased()
{
	UE_LOG(LogTemp, Warning, TEXT("PACKAGE"));
}
