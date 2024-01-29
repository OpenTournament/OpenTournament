// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "SubtitleDisplayOptions.h"

#include "SubtitleDisplaySubsystem.generated.h"

class FSubsystemCollectionBase;
class ULocalPlayer;
class UObject;

USTRUCT(BlueprintType)
struct GAMESUBTITLES_API FSubtitleFormat
{
	GENERATED_BODY()

public:
	FSubtitleFormat()
		: SubtitleTextSize(ESubtitleDisplayTextSize::Medium)
		, SubtitleTextColor(ESubtitleDisplayTextColor::White)
		, SubtitleTextBorder(ESubtitleDisplayTextBorder::None)
		, SubtitleBackgroundOpacity(ESubtitleDisplayBackgroundOpacity::Medium)
	{
	}

public:
	UPROPERTY(EditAnywhere, Category = "Display Info")
	ESubtitleDisplayTextSize SubtitleTextSize;

	UPROPERTY(EditAnywhere, Category = "Display Info")
	ESubtitleDisplayTextColor SubtitleTextColor;

	UPROPERTY(EditAnywhere, Category = "Display Info")
	ESubtitleDisplayTextBorder SubtitleTextBorder;

	UPROPERTY(EditAnywhere, Category = "Display Info")
	ESubtitleDisplayBackgroundOpacity SubtitleBackgroundOpacity;
};

UCLASS(DisplayName = "Subtitle Display")
class GAMESUBTITLES_API USubtitleDisplaySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	DECLARE_EVENT_OneParam(USubtitleDisplaySubsystem, FDisplayFormatChangedEvent, const FSubtitleFormat& /*DisplayFormat*/);
	FDisplayFormatChangedEvent DisplayFormatChangedEvent;

public:
	static USubtitleDisplaySubsystem* Get(const ULocalPlayer* LocalPlayer);

public:
	USubtitleDisplaySubsystem();

	// Begin USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// End USubsystem

	void SetSubtitleDisplayOptions(const FSubtitleFormat& InOptions);
	const FSubtitleFormat& GetSubtitleDisplayOptions() const;

private:
	UPROPERTY()
	FSubtitleFormat SubtitleFormat;
};
