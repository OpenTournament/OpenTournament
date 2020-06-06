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

#include "Slate/Styles.h"

#include "Styling/SlateBrush.h"

// ---------------------------------------------------------------------------
//		FStyles - General
// ---------------------------------------------------------------------------

float FStyles::GetBlockHeight()
{
	return 35.0f;
}

FMargin FStyles::GetBlockPadding()
{
	return FMargin(25.0f, 0.0f, 25.0f, 0.0f);
}

// ---------------------------------------------------------------------------
//		FStyles - Color
// ---------------------------------------------------------------------------

FLinearColor FStyles::GetColorNone()
{
	return FLinearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

FLinearColor FStyles::GetColorBlack()
{
	return FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

FLinearColor FStyles::GetColorGreyLight()
{
	return FLinearColor(0.15f, 0.15f, 0.15f, 1.0f);
}

FLinearColor FStyles::GetColorGreyMedium()
{
	return FLinearColor(0.1f, 0.1f, 0.1f, 1.0f);
}

FLinearColor FStyles::GetColorGreyDark()
{
	return FLinearColor(0.05f, 0.05f, 0.05f, 1.0f);
}

FLinearColor FStyles::GetColorGreen()
{
	return FLinearColor(0.0f, 0.1f, 0.0f, 1.0f);
}

// ---------------------------------------------------------------------------
//		FStyles - Margin
// ---------------------------------------------------------------------------

FMargin FStyles::GetMarginAll()
{
	return FMargin(1.0f, 1.0f, 1.0f, 1.0f);
}

FMargin FStyles::GetMarginHorizontal()
{
	return FMargin(0.0f, 1.0f, 0.0f, 1.0f);
}

FMargin FStyles::GetMarginVertical()
{
	return FMargin(1.0f, 0.0f, 1.0f, 0.0f);
}

FMargin FStyles::GetMarginNone()
{
	return FMargin(0.0f, 0.0f, 0.0f, 0.0f);
}

FMargin FStyles::GetMarginTop()
{
	return FMargin(0.0f, 1.0f, 0.0f, 0.0f);
}

FMargin FStyles::GetMarginBottom()
{
	return FMargin(0.0f, 0.0f, 0.0f, 1.0f);
}

FMargin FStyles::GetMarginLeft()
{
	return FMargin(1.0f, 0.0f, 0.0f, 0.0f);
}

FMargin FStyles::GetMarginRight()
{
	return FMargin(0.0f, 0.0f, 1.0f, 0.0f);
}

// ---------------------------------------------------------------------------
//		FStyles - Padding
// ---------------------------------------------------------------------------

FMargin FStyles::GetPaddingAll()
{
	return FMargin(5.0f, 5.0f, 5.0f, 5.0f);
}

FMargin FStyles::GetPaddingVertical()
{
	return FMargin(0.0f, 5.0f, 0.0f, 5.0f);
}

FMargin FStyles::GetPaddingHorizontal()
{
	return FMargin(5.0f, 0.0f, 5.0f, 0.0f);
}

FMargin FStyles::GetPaddingNone()
{
	return FMargin(0.0f, 0.0f, 0.0f, 0.0f);
}

// ---------------------------------------------------------------------------
//		FStyles - Brush
// ---------------------------------------------------------------------------

FSlateBrush* FStyles::GetBackgroundNone()
{
	FSlateBrush* SlateBrush = new FSlateBrush();
	SlateBrush->DrawAs = ESlateBrushDrawType::NoDrawType;
	return SlateBrush;
}

FSlateBrush* FStyles::GetBackgroundBlack()
{
	FSlateBrush* SlateBrush = new FSlateBrush();
	SlateBrush->DrawAs = ESlateBrushDrawType::Image;
	SlateBrush->Margin = GetMarginNone();
	SlateBrush->TintColor = GetColorBlack();
	return SlateBrush;
}

FSlateBrush* FStyles::GetBackgroundGreyLight()
{
	FSlateBrush* SlateBrush = new FSlateBrush();
	SlateBrush->DrawAs = ESlateBrushDrawType::Image;
	SlateBrush->Margin = GetMarginNone();
	SlateBrush->TintColor = GetColorGreyLight();
	return SlateBrush;
}

FSlateBrush* FStyles::GetBackgroundGreyMedium()
{
	FSlateBrush* SlateBrush = new FSlateBrush();
	SlateBrush->DrawAs = ESlateBrushDrawType::Image;
	SlateBrush->Margin = GetMarginNone();
	SlateBrush->TintColor = GetColorGreyMedium();
	return SlateBrush;
}

FSlateBrush* FStyles::GetBackgroundGreyDark()
{
	FSlateBrush* SlateBrush = new FSlateBrush();
	SlateBrush->DrawAs = ESlateBrushDrawType::Image;
	SlateBrush->Margin = GetMarginNone();
	SlateBrush->TintColor = GetColorGreyDark();
	return SlateBrush;
}

FSlateBrush* FStyles::GetBackgroundGreen()
{
	FSlateBrush* SlateBrush = new FSlateBrush();
	SlateBrush->DrawAs = ESlateBrushDrawType::Image;
	SlateBrush->Margin = FMargin(0.0f, 0.0f, 0.0f, 0.0f);
	SlateBrush->TintColor = GetColorGreen();
	return SlateBrush;
}

FSlateBrush* FStyles::GetBorderGreenAll()
{
	FSlateBrush* SlateBrush = new FSlateBrush();
	SlateBrush->DrawAs = ESlateBrushDrawType::Border;
	SlateBrush->Margin = GetMarginAll();
	SlateBrush->TintColor = GetColorGreen();
	return SlateBrush;
}

FSlateBrush* FStyles::GetBorderGreenHorizontal()
{
	FSlateBrush* SlateBrush = new FSlateBrush();
	SlateBrush->DrawAs = ESlateBrushDrawType::Border;
	SlateBrush->Margin = GetMarginHorizontal();
	SlateBrush->TintColor = GetColorGreen();
	return SlateBrush;
}

FSlateBrush* FStyles::GetBorderGreenVertical()
{
	FSlateBrush* SlateBrush = new FSlateBrush();
	SlateBrush->DrawAs = ESlateBrushDrawType::Border;
	SlateBrush->Margin = GetMarginVertical();
	SlateBrush->TintColor = GetColorGreen();
	return SlateBrush;
}

FSlateBrush* FStyles::GetBorderGreenTop()
{
	FSlateBrush* SlateBrush = new FSlateBrush();
	SlateBrush->DrawAs = ESlateBrushDrawType::Border;
	SlateBrush->Margin = GetMarginTop();
	SlateBrush->TintColor = GetColorGreen();
	return SlateBrush;
}

FSlateBrush* FStyles::GetBorderGreenBottom()
{
	FSlateBrush* SlateBrush = new FSlateBrush();
	SlateBrush->DrawAs = ESlateBrushDrawType::Border;
	SlateBrush->Margin = GetMarginBottom();
	SlateBrush->TintColor = GetColorGreen();
	return SlateBrush;
}

FSlateBrush* FStyles::GetBorderGreenLeft()
{
	FSlateBrush* SlateBrush = new FSlateBrush();
	SlateBrush->DrawAs = ESlateBrushDrawType::Border;
	SlateBrush->Margin = GetMarginLeft();
	SlateBrush->TintColor = GetColorGreen();
	return SlateBrush;
}

FSlateBrush* FStyles::GetBorderGreenRight()
{
	FSlateBrush* SlateBrush = new FSlateBrush();
	SlateBrush->DrawAs = ESlateBrushDrawType::Border;
	SlateBrush->Margin = GetMarginRight();
	SlateBrush->TintColor = GetColorGreen();
	return SlateBrush;
}

// ---------------------------------------------------------------------------
