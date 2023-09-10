// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidgetPool.h"

#include "GameSettingDetailView.generated.h"

class UCommonRichTextBlock;
class UCommonTextBlock;
class UGameSetting;
class UGameSettingDetailExtension;
class UGameSettingVisualData;
class UObject;
class UVerticalBox;
struct FStreamableHandle;

/**
 * 
 */
UCLASS(Abstract)
class GAMESETTINGS_API UGameSettingDetailView : public UUserWidget
{
	GENERATED_BODY()
public:
	UGameSettingDetailView(const FObjectInitializer& ObjectInitializer);

	void FillSettingDetails(UGameSetting* InSetting);

	//UVisual interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	//~ End UVisual Interface

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;

	void CreateDetailsExtension(UGameSetting* InSetting, TSubclassOf<UGameSettingDetailExtension> ExtensionClass);

protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UGameSettingVisualData> VisualData;

	UPROPERTY(Transient)
	FUserWidgetPool ExtensionWidgetPool;

	UPROPERTY(Transient)
	TObjectPtr<UGameSetting> CurrentSetting;

	TSharedPtr<FStreamableHandle> StreamingHandle;

private:	// Bound Widgets
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UCommonTextBlock> Text_SettingName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UCommonRichTextBlock> RichText_Description;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UCommonRichTextBlock> RichText_DynamicDetails;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UCommonRichTextBlock> RichText_WarningDetails;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UCommonRichTextBlock> RichText_DisabledDetails;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UVerticalBox> Box_DetailsExtension;
};
