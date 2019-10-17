// Copyright (c) 2019 Open Tournament Project, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

#include "DeclarativeSyntaxSupport.h"

/**
 * 
 */
class OPENTOURNAMENT_API SUR_LoadingScreenWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SUR_LoadingScreenWidget)
	{}
	SLATE_ARGUMENT(FString, MapName)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	static TSharedRef<SWidget> Create(const FString& MapName)
	{
		return SNew(SUR_LoadingScreenWidget).MapName(MapName);
	}
};
