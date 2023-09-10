// Copyright Epic Games, Inc. All Rights Reserved.


#include "Widgets/GameSettingDetailView.h"

#include "CommonRichTextBlock.h"
#include "CommonTextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "GameSetting.h"
#include "Widgets/GameSettingDetailExtension.h"
#include "Widgets/GameSettingVisualData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameSettingDetailView)

#define LOCTEXT_NAMESPACE "GameSetting"

UGameSettingDetailView::UGameSettingDetailView(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ExtensionWidgetPool(*this)
{
}

void UGameSettingDetailView::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	ExtensionWidgetPool.ReleaseAllSlateResources();
}

void UGameSettingDetailView::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (!IsDesignTime())
	{
		FillSettingDetails(nullptr);
	}
}

void UGameSettingDetailView::NativeConstruct()
{
	Super::NativeConstruct();
}

void UGameSettingDetailView::FillSettingDetails(UGameSetting* InSetting)
{
	// Ignore requests to show the same setting multiple times in a row.
	if (InSetting && InSetting == CurrentSetting)
	{
		return;
	}

	CurrentSetting = InSetting;

	if (Text_SettingName)
	{
		Text_SettingName->SetText(InSetting ? InSetting->GetDisplayName() : FText::GetEmpty());
	}

	if (RichText_Description)
	{
		RichText_Description->SetText(InSetting ? InSetting->GetDescriptionRichText() : FText::GetEmpty());
	}

	if (RichText_DynamicDetails)
	{
		const FText DynamicDetails = InSetting ? InSetting->GetDynamicDetails() : FText::GetEmpty();
		RichText_DynamicDetails->SetText(DynamicDetails);
		RichText_DynamicDetails->SetVisibility(DynamicDetails.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}

	if (RichText_WarningDetails)
	{
		if (InSetting && !InSetting->GetWarningRichText().IsEmpty())
		{
			const FText WarningText = FText::Format(LOCTEXT("WarningReasonLine", "<Icon.Warning></> {0}"), InSetting->GetWarningRichText());
			RichText_WarningDetails->SetText(WarningText);
			RichText_WarningDetails->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			RichText_WarningDetails->SetText(FText::GetEmpty());
			RichText_WarningDetails->SetVisibility(ESlateVisibility::Collapsed);
		}

	}

	if (RichText_DisabledDetails)
	{
		TArray<FText> DisabledDetailLines;

		if (InSetting)
		{
			FGameSettingEditableState EditableState = InSetting->GetEditState();

			if (!EditableState.IsEnabled())
			{
				for (FText Reason : EditableState.GetDisabledReasons())
				{
					DisabledDetailLines.Add(FText::Format(LOCTEXT("DisabledReasonLine", "<Icon.Warning></> {0}"), Reason));
				}
			}

			if (EditableState.GetDisabledOptions().Num() > 0)
			{
				DisabledDetailLines.Add(LOCTEXT("DisabledOptionReasonLine", "<Icon.Warning></> There are fewer options than available due to Parental Controls."));
			}
		}

		RichText_DisabledDetails->SetText(FText::Join(FText::FromString(TEXT("\n")), DisabledDetailLines));
		RichText_DisabledDetails->SetVisibility(DisabledDetailLines.Num() == 0 ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
	
	if (Box_DetailsExtension)
	{
		// First release the widgets back into the pool.
		for (UWidget* ChildExtension : Box_DetailsExtension->GetAllChildren())
		{
			ExtensionWidgetPool.Release(Cast<UUserWidget>(ChildExtension));
		}

		// Remove the widgets from their container.
		Box_DetailsExtension->ClearChildren();

		if (InSetting)
		{
			TArray<TSoftClassPtr<UGameSettingDetailExtension>> ExtensionClassPtrs;
			if (VisualData)
			{
				ExtensionClassPtrs = VisualData->GatherDetailExtensions(InSetting);
			}
			
			if (StreamingHandle.IsValid())
			{
				StreamingHandle->CancelHandle();
			}

			bool bEverythingAlreadyLoaded = true;

			TArray<FSoftObjectPath> ExtensionPaths;
			ExtensionPaths.Reserve(ExtensionClassPtrs.Num());
			for (TSoftClassPtr<UGameSettingDetailExtension> SoftClassPtr : ExtensionClassPtrs)
			{
				bEverythingAlreadyLoaded &= SoftClassPtr.IsValid();
				ExtensionPaths.Add(SoftClassPtr.ToSoftObjectPath());
			}

			if (bEverythingAlreadyLoaded)
			{
				for (TSoftClassPtr<UGameSettingDetailExtension> SoftClassPtr : ExtensionClassPtrs)
				{
					CreateDetailsExtension(InSetting, SoftClassPtr.Get());
				}

				ExtensionWidgetPool.ReleaseInactiveSlateResources();
			}
			else
			{
				TWeakObjectPtr<UGameSetting> SettingPtr = InSetting;

				StreamingHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
					MoveTemp(ExtensionPaths),
					FStreamableDelegate::CreateWeakLambda(this, [this, SettingPtr, ExtensionClassPtrs] {
						for (TSoftClassPtr<UGameSettingDetailExtension> SoftClassPtr : ExtensionClassPtrs)
						{
							CreateDetailsExtension(SettingPtr.Get(), SoftClassPtr.Get());
						}

						ExtensionWidgetPool.ReleaseInactiveSlateResources();
					}
				));
			}
		}
	}
}

void UGameSettingDetailView::CreateDetailsExtension(UGameSetting* InSetting, TSubclassOf<UGameSettingDetailExtension> ExtensionClass)
{
	if (InSetting && ExtensionClass)
	{
		if (UGameSettingDetailExtension* Extension = ExtensionWidgetPool.GetOrCreateInstance(ExtensionClass))
		{
			Extension->SetSetting(InSetting);
			UVerticalBoxSlot* ExtensionSlot = Box_DetailsExtension->AddChildToVerticalBox(Extension);
			ExtensionSlot->SetHorizontalAlignment(HAlign_Fill);
		}
	}
}

#undef LOCTEXT_NAMESPACE
