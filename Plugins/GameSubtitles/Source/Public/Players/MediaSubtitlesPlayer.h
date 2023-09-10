// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Tickable.h"

#include "UObject/ObjectPtr.h"
#include "UObject/WeakObjectPtr.h"
#include "MediaSubtitlesPlayer.generated.h"

class UMediaPlayer;
class UOverlays;
struct FFrame;

/**
 * A Game-specific player for media subtitles. This needs to exist next to Media Players
 * and have its Play() / Pause() / Stop() methods called at the same time as the media players'
 * methods.
 */
UCLASS(BlueprintType)
class GAMESUBTITLES_API UMediaSubtitlesPlayer
	: public UObject
	, public FTickableGameObject
{
	GENERATED_UCLASS_BODY()

public:

	/** The subtitles to use for this player. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Subtitles Source")
	TObjectPtr<UOverlays> SourceSubtitles;

public:

	virtual void BeginDestroy() override;

	/** Begins playing the currently set subtitles. */
	UFUNCTION(BlueprintCallable, Category="Game Subtitles|Subtitles Player")
	void Play();

	/** Stops the subtitle player. */
	UFUNCTION(BlueprintCallable, Category="Game Subtitles|Subtitles Player")
	void Stop();

	/** Sets the source with the new subtitles set. */
	UFUNCTION(BlueprintCallable, Category="Game Subtitles|Subtitles Player")
	void SetSubtitles(UOverlays* Subtitles);

	/** Binds the subtitle playback to the tick of a media player. */
	UFUNCTION(BlueprintCallable, Category="Game Subtitles|Subtitles Player")
	void BindToMediaPlayer(UMediaPlayer* InMediaPlayer);

public:

	//~ FTickableGameObject interface
	virtual void Tick(float DeltaSeconds) override;
	virtual ETickableTickType GetTickableTickType() const override { return (HasAnyFlags(RF_ClassDefaultObject) ? ETickableTickType::Never : ETickableTickType::Always); }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UMediaSubtitlesPlayer, STATGROUP_Tickables); }

private:

	/** A reference to our media player */
	TWeakObjectPtr<class UMediaPlayer> MediaPlayer;

	/** Whether the subtitles are currently being displayed */
	bool bEnabled;
};
