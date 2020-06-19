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

#include "Slate/ButtonUMods.h"
#include "Slate/Styles.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"

void SButtonUMods::Construct(const FArguments& InArgs)
{
	FButtonStyle* ButtonStyle = new FButtonStyle();
	ButtonStyle->SetNormalPadding(FStyles::GetPaddingNone());
	ButtonStyle->SetPressedPadding(FStyles::GetPaddingNone());
	Style = ButtonStyle;
	OnReleased.BindRaw(this, &SButtonUMods::OnSelectedInternal);
	ContentPadding = FStyles::GetPaddingNone();
	DisabledImage = FStyles::GetBackgroundNone();
	PressedImage = FStyles::GetBackgroundNone();
	HoverImage = FStyles::GetBackgroundNone();
	NormalImage = FStyles::GetBackgroundNone();
	TSharedRef<SBox> Box = SNew(SBox);
	{
		TSharedRef<SBorder> Border = SNew(SBorder);
		{
			TextBlock->SetColorAndOpacity(FLinearColor(0.25f, 0.25f, 0.25f, 1));
			TextBlock->SetText(FText::FromString(TEXT("Default")));
			TextBlock->SetJustification(ETextJustify::Center);

			Border->SetVAlign(VAlign_Center);
			Border->SetBorderImage(FStyles::GetBackgroundNone());
			Border->SetContent(TextBlock);
		}
		Box->SetPadding(FStyles::GetPaddingNone());
		Box->SetHeightOverride(FStyles::GetBlockHeight());
		Box->SetContent(Border);
	}
	ChildSlot.AttachWidget(Box);
}

FReply SButtonUMods::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	TextBlock->SetColorAndOpacity(FLinearColor(1, 1, 1, 1));
	return SButton::OnMouseButtonDown(MyGeometry, MouseEvent);;
}

FReply SButtonUMods::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	TextBlock->SetColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f, 1));
	return SButton::OnMouseButtonUp(MyGeometry, MouseEvent);;
}

void SButtonUMods::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	SButton::OnMouseEnter(MyGeometry, MouseEvent);
	TextBlock->SetColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f, 1));
}

void SButtonUMods::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	SButton::OnMouseLeave(MouseEvent);
	if (!IsPressed())
	{
		TextBlock->SetColorAndOpacity(FLinearColor(0.25f, 0.25f, 0.25f, 1));
	}
}

void SButtonUMods::OnSelectedInternal()
{
	OnSelected.ExecuteIfBound();
}
