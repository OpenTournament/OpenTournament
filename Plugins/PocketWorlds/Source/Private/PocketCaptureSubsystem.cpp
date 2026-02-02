// Copyright Epic Games, Inc. All Rights Reserved.

#include "PocketCaptureSubsystem.h"

#include "Components/PrimitiveComponent.h"
#include "PocketCapture.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PocketCaptureSubsystem)

class FSubsystemCollectionBase;

// UPocketCaptureSubsystem
//---------------------------------------------------------------------------------

UPocketCaptureSubsystem::UPocketCaptureSubsystem()
{
}

void UPocketCaptureSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &ThisClass::Tick));
}

void UPocketCaptureSubsystem::Deinitialize()
{
	FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);

	for (int32 RendererIndex = 0; RendererIndex < ThumbnailRenderers.Num(); RendererIndex++)
	{
		if (UPocketCapture* Renderer = ThumbnailRenderers[RendererIndex].Get())
		{
			Renderer->Deinitialize();
		}
	}

	ThumbnailRenderers.Reset();
}

UPocketCapture* UPocketCaptureSubsystem::CreateThumbnailRenderer(TSubclassOf<UPocketCapture> ThumbnailRendererClass)
{
	UPocketCapture* Renderer = NewObject<UPocketCapture>(this, ThumbnailRendererClass);

	int32 RendererEmptyIndex = ThumbnailRenderers.IndexOfByKey(nullptr);
	if (RendererEmptyIndex == INDEX_NONE)
	{
		RendererEmptyIndex = ThumbnailRenderers.Add(Renderer);
	}
	else
	{
		ThumbnailRenderers[RendererEmptyIndex] = Renderer;
	}

	Renderer->Initialize(GetWorld(), RendererEmptyIndex);

	return Renderer;
}

void UPocketCaptureSubsystem::DestroyThumbnailRenderer(UPocketCapture* ThumbnailRenderer)
{
	if (ThumbnailRenderer)
	{
		const int32 ThumbnailIndex = ThumbnailRenderers.IndexOfByKey(ThumbnailRenderer);
		if (ThumbnailIndex != INDEX_NONE)
		{
			ThumbnailRenderers[ThumbnailIndex] = nullptr;
			ThumbnailRenderer->Deinitialize();
		}
	}
}

void UPocketCaptureSubsystem::StreamThisFrame(TArray<UPrimitiveComponent*>& PrimitiveComponents)
{
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		PrimitiveComponent->bForceMipStreaming = true;
		StreamedLastFrameButNotNext.Remove(PrimitiveComponent);
	}

	StreamNextFrame.Append(PrimitiveComponents);
}

bool UPocketCaptureSubsystem::Tick(float DeltaTime)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_URealTimeThumbnailSubsystem_Tick);

	for (TWeakObjectPtr<UPrimitiveComponent> PrimitiveComponent : StreamedLastFrameButNotNext)
	{
		if (PrimitiveComponent.IsValid())
		{
			PrimitiveComponent->bForceMipStreaming = false;
		}
	}

	StreamedLastFrameButNotNext = MoveTemp(StreamNextFrame);

	return true;
}

