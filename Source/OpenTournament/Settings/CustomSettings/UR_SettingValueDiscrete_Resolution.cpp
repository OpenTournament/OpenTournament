// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_SettingValueDiscrete_Resolution.h"

#include "Framework/Application/SlateApplication.h"
#include "GameFramework/GameUserSettings.h"
#include "RHI.h"
#include "UnrealEngine.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_SettingValueDiscrete_Resolution)

/////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "OTSettings"

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_SettingValueDiscrete_Resolution::UUR_SettingValueDiscrete_Resolution()
{}

void UUR_SettingValueDiscrete_Resolution::OnInitialized()
{
    Super::OnInitialized();

    InitializeResolutions();
}

void UUR_SettingValueDiscrete_Resolution::StoreInitial()
{
    // Ignored
}

void UUR_SettingValueDiscrete_Resolution::ResetToDefault()
{
    // Ignored
}

void UUR_SettingValueDiscrete_Resolution::RestoreToInitial()
{
    // Ignored
}

void UUR_SettingValueDiscrete_Resolution::SetDiscreteOptionByIndex(int32 Index)
{
    if (Resolutions.IsValidIndex(Index) && Resolutions[Index].IsValid())
    {
        GEngine->GetGameUserSettings()->SetScreenResolution(Resolutions[Index]->GetResolution());
        NotifySettingChanged(EGameSettingChangeReason::Change);
    }
}

int32 UUR_SettingValueDiscrete_Resolution::GetDiscreteOptionIndex() const
{
    const UGameUserSettings* UserSettings = CastChecked<const UGameUserSettings>(GEngine->GetGameUserSettings());

    return FindIndexOfDisplayResolutionForceValid(UserSettings->GetScreenResolution());
}

TArray<FText> UUR_SettingValueDiscrete_Resolution::GetDiscreteOptions() const
{
    TArray<FText> ReturnResolutionTexts;

    for (int32 i = 0; i < Resolutions.Num(); ++i)
    {
        ReturnResolutionTexts.Add(Resolutions[i]->GetDisplayText());
    }

    return ReturnResolutionTexts;
}

void UUR_SettingValueDiscrete_Resolution::OnDependencyChanged()
{
    const FIntPoint CurrentResolution = GEngine->GetGameUserSettings()->GetScreenResolution();
    SelectAppropriateResolutions();
    SetDiscreteOptionByIndex(FindClosestResolutionIndex(CurrentResolution));
}

void UUR_SettingValueDiscrete_Resolution::InitializeResolutions()
{
    Resolutions.Empty();
    ResolutionsFullscreen.Empty();
    ResolutionsWindowed.Empty();
    ResolutionsWindowedFullscreen.Empty();

    FDisplayMetrics InitialDisplayMetrics;
    FSlateApplication::Get().GetInitialDisplayMetrics(InitialDisplayMetrics);

    FScreenResolutionArray ResArray;
    RHIGetAvailableResolutions(ResArray, true);

    // Determine available windowed modes
    {
        TArray<FIntPoint> WindowedResolutions;
        const FIntPoint MinResolution(1280, 720);
        // Use the primary display resolution minus 1 to exclude the primary display resolution from the list.
        // This is so you don't make a window so large that part of the game is off screen and you are unable to change resolutions back.
        const FIntPoint MaxResolution(InitialDisplayMetrics.PrimaryDisplayWidth - 1, InitialDisplayMetrics.PrimaryDisplayHeight - 1);
        // Excluding 4:3 and below
        const float MinAspectRatio = 16 / 10.f;

        if (MaxResolution.X >= MinResolution.X && MaxResolution.Y >= MinResolution.Y)
        {
            GetStandardWindowResolutions(MinResolution, MaxResolution, MinAspectRatio, WindowedResolutions);
        }

        if (GSystemResolution.WindowMode == EWindowMode::Windowed)
        {
            WindowedResolutions.AddUnique(FIntPoint(GSystemResolution.ResX, GSystemResolution.ResY));
            WindowedResolutions.Sort
            ([](const FIntPoint& A, const FIntPoint& B)
            {
                return A.X != B.X ? A.X < B.X : A.Y < B.Y;
            });
        }

        // If there were no standard resolutions. Add the primary display size, just so one exists.
        // This might happen if we are running on a non-standard device.
        if (WindowedResolutions.Num() == 0)
        {
            WindowedResolutions.Add(FIntPoint(InitialDisplayMetrics.PrimaryDisplayWidth, InitialDisplayMetrics.PrimaryDisplayHeight));
        }

        ResolutionsWindowed.Empty(WindowedResolutions.Num());
        for (const FIntPoint& Res : WindowedResolutions)
        {
            TSharedRef<FScreenResolutionEntry> Entry = MakeShared<FScreenResolutionEntry>();
            Entry->Width = Res.X;
            Entry->Height = Res.Y;

            ResolutionsWindowed.Add(Entry);
        }
    }

    // Determine available windowed full-screen modes
    {
        FScreenResolutionRHI* RHIInitialResolution = ResArray.FindByPredicate
        ([InitialDisplayMetrics](const FScreenResolutionRHI& ScreenRes)
        {
            return ScreenRes.Width == InitialDisplayMetrics.PrimaryDisplayWidth && ScreenRes.Height == InitialDisplayMetrics.PrimaryDisplayHeight;
        });

        TSharedRef<FScreenResolutionEntry> Entry = MakeShared<FScreenResolutionEntry>();
        if (RHIInitialResolution)
        {
            // If this is in the official list use that
            Entry->Width = RHIInitialResolution->Width;
            Entry->Height = RHIInitialResolution->Height;
            Entry->RefreshRate = RHIInitialResolution->RefreshRate;
        }
        else
        {
            // Custom resolution the RHI doesn't expect
            Entry->Width = InitialDisplayMetrics.PrimaryDisplayWidth;
            Entry->Height = InitialDisplayMetrics.PrimaryDisplayHeight;

            // TODO: Unsure how to calculate refresh rate
            Entry->RefreshRate = FPlatformMisc::GetMaxRefreshRate();
        }

        ResolutionsWindowedFullscreen.Add(Entry);
    }

    // Determine available full-screen modes
    if (ResArray.Num() > 0)
    {
        // try more strict first then more relaxed, we want at least one resolution to remain
        for (int32 FilterThreshold = 0; FilterThreshold < 3; ++FilterThreshold)
        {
            for (int32 ModeIndex = 0; ModeIndex < ResArray.Num(); ModeIndex++)
            {
                const FScreenResolutionRHI& ScreenRes = ResArray[ModeIndex];

                // first try with struct test, than relaxed test
                if (ShouldAllowFullScreenResolution(ScreenRes, FilterThreshold))
                {
                    TSharedRef<FScreenResolutionEntry> Entry = MakeShared<FScreenResolutionEntry>();
                    Entry->Width = ScreenRes.Width;
                    Entry->Height = ScreenRes.Height;
                    Entry->RefreshRate = ScreenRes.RefreshRate;

                    ResolutionsFullscreen.Add(Entry);
                }
            }

            if (ResolutionsFullscreen.Num())
            {
                // we found some resolutions, otherwise we try with more relaxed tests
                break;
            }
        }
    }

    SelectAppropriateResolutions();
}

void UUR_SettingValueDiscrete_Resolution::SelectAppropriateResolutions()
{
    EWindowMode::Type const WindowMode = GEngine->GetGameUserSettings()->GetFullscreenMode();
    if (LastWindowMode != WindowMode)
    {
        LastWindowMode = WindowMode;

        Resolutions.Empty();
        switch (WindowMode)
        {
            case EWindowMode::Windowed:
                Resolutions.Append(ResolutionsWindowed);
                break;
            case EWindowMode::WindowedFullscreen:
                Resolutions.Append(ResolutionsWindowedFullscreen);
                break;
            case EWindowMode::Fullscreen:
                Resolutions.Append(ResolutionsFullscreen);
                break;
        }

        NotifyEditConditionsChanged();
    }
}

// To filter out odd resolution so UI and testing has less issues. This is game specific.
// @param ScreenRes resolution and
// @param FilterThreshold 0/1/2 to make sure we get at least some resolutions (might be an issues with UI but at least we get some resolution entries)
bool UUR_SettingValueDiscrete_Resolution::ShouldAllowFullScreenResolution(const FScreenResolutionRHI& SrcScreenRes, int32 FilterThreshold) const
{
    FScreenResolutionRHI ScreenRes = SrcScreenRes;

    // expected: 4:3=1.333, 16:9=1.777, 16:10=1.6, multi-monitor-wide: >2
    bool bIsPortrait = ScreenRes.Width < ScreenRes.Height;
    float AspectRatio = (float)ScreenRes.Width / (float)ScreenRes.Height;

    // If portrait, flip values back to landscape so we can don't have to special case all the tests below
    if (bIsPortrait)
    {
        AspectRatio = 1.0f / AspectRatio;
        ScreenRes.Width = SrcScreenRes.Height;
        ScreenRes.Height = SrcScreenRes.Width;
    }

    // Filter out resolutions that don't match the native aspect ratio of the primary monitor
    // TODO: Other games allow the user to choose which monitor the games goes fullscreen on. This would allow
    // this filtering to be correct when the users monitors are of different types! ATM, the game can change
    // which monitor it uses based on other factors (max window overlap etc.) so we could end up choosing a
    // resolution which the target monitor doesn't support.
    if (FilterThreshold < 1)
    {
        FDisplayMetrics DisplayMetrics;
        FSlateApplication::Get().GetInitialDisplayMetrics(DisplayMetrics);

        // Default display aspect to required aspect in case this platform can't provide the information. Forces acceptance of this resolution.
        float DisplayAspect = AspectRatio;

        // Some platforms might not be able to detect the native resolution of the display device, so don't filter in that case
        for (int32 MonitorIndex = 0; MonitorIndex < DisplayMetrics.MonitorInfo.Num(); ++MonitorIndex)
        {
            FMonitorInfo& MonitorInfo = DisplayMetrics.MonitorInfo[MonitorIndex];

            if (MonitorInfo.bIsPrimary)
            {
                DisplayAspect = (float)MonitorInfo.NativeWidth / (float)MonitorInfo.NativeHeight;
                break;
            }
        }

        // If aspects are not almost exactly equal, reject
        if (FMath::Abs(DisplayAspect - AspectRatio) > KINDA_SMALL_NUMBER)
        {
            return false;
        }
    }

    // more relaxed tests have a larger FilterThreshold

    // minimum is 1280x720
    if (FilterThreshold < 2 && (ScreenRes.Width < 1280 || ScreenRes.Height < 720))
    {
        // filter resolutions that are too small
        return false;
    }

    return true;
}

int32 UUR_SettingValueDiscrete_Resolution::FindIndexOfDisplayResolution(const FIntPoint& InPoint) const
{
    // find the current res
    for (int32 i = 0, Num = Resolutions.Num(); i < Num; ++i)
    {
        if (Resolutions[i]->GetResolution() == InPoint)
        {
            return i;
        }
    }

    return INDEX_NONE;
}

int32 UUR_SettingValueDiscrete_Resolution::FindIndexOfDisplayResolutionForceValid(const FIntPoint& InPoint) const
{
    int32 Result = FindIndexOfDisplayResolution(InPoint);
    if (Result == INDEX_NONE && Resolutions.Num() > 0)
    {
        Result = Resolutions.Num() - 1;
    }

    return Result;
}

int32 UUR_SettingValueDiscrete_Resolution::FindClosestResolutionIndex(const FIntPoint& Resolution) const
{
    int32 Index = 0;
    int32 LastDiff = Resolution.SizeSquared();

    for (int32 i = 0, Num = Resolutions.Num(); i < Num; ++i)
    {
        // We compare the squared diagonals
        int32 Diff = FMath::Abs(Resolution.SizeSquared() - Resolutions[i]->GetResolution().SizeSquared());
        if (Diff <= LastDiff)
        {
            Index = i;
        }
        LastDiff = Diff;
    }

    return Index;
}

void UUR_SettingValueDiscrete_Resolution::GetStandardWindowResolutions(const FIntPoint& MinResolution, const FIntPoint& MaxResolution, float MinAspectRatio, TArray<FIntPoint>& OutResolutions)
{
    static TArray<FIntPoint> StandardResolutions;
    if (StandardResolutions.Num() == 0)
    {
        // Standard resolutions as provided by Wikipedia (http://en.wikipedia.org/wiki/Graphics_display_resolution)

        // Extended Graphics Array
        {
            new(StandardResolutions) FIntPoint(1024, 768); // XGA

            // WXGA (3 versions)
            new(StandardResolutions) FIntPoint(1366, 768); // FWXGA
            new(StandardResolutions) FIntPoint(1360, 768);
            new(StandardResolutions) FIntPoint(1280, 800);

            new(StandardResolutions) FIntPoint(1152, 864); // XGA+
            new(StandardResolutions) FIntPoint(1440, 900); // WXGA+
            new(StandardResolutions) FIntPoint(1280, 1024); // SXGA
            new(StandardResolutions) FIntPoint(1400, 1050); // SXGA+
            new(StandardResolutions) FIntPoint(1680, 1050); // WSXGA+
            new(StandardResolutions) FIntPoint(1600, 1200); // UXGA
            new(StandardResolutions) FIntPoint(1920, 1200); // WUXGA
        }

        // Quad Extended Graphics Array
        {
            new(StandardResolutions) FIntPoint(2048, 1152); // QWXGA
            new(StandardResolutions) FIntPoint(2048, 1536); // QXGA
            new(StandardResolutions) FIntPoint(2560, 1600); // WQXGA
            new(StandardResolutions) FIntPoint(2560, 2048); // QSXGA
            new(StandardResolutions) FIntPoint(3200, 2048); // WQSXGA
            new(StandardResolutions) FIntPoint(3200, 2400); // QUXGA
            new(StandardResolutions) FIntPoint(3840, 2400); // WQUXGA
        }

        // Hyper Extended Graphics Array
        {
            new(StandardResolutions) FIntPoint(4096, 3072); // HXGA
            new(StandardResolutions) FIntPoint(5120, 3200); // WHXGA
            new(StandardResolutions) FIntPoint(5120, 4096); // HSXGA
            new(StandardResolutions) FIntPoint(6400, 4096); // WHSXGA
            new(StandardResolutions) FIntPoint(6400, 4800); // HUXGA
            new(StandardResolutions) FIntPoint(7680, 4800); // WHUXGA
        }

        // High-Definition
        {
            new(StandardResolutions) FIntPoint(640, 360); // nHD
            new(StandardResolutions) FIntPoint(960, 540); // qHD
            new(StandardResolutions) FIntPoint(1280, 720); // HD
            new(StandardResolutions) FIntPoint(1920, 1080); // FHD
            new(StandardResolutions) FIntPoint(2560, 1440); // QHD
            new(StandardResolutions) FIntPoint(3200, 1800); // WQXGA+
            new(StandardResolutions) FIntPoint(3840, 2160); // UHD 4K
            new(StandardResolutions) FIntPoint(4096, 2160); // Digital Cinema Initiatives 4K
            new(StandardResolutions) FIntPoint(7680, 4320); // FUHD
            new(StandardResolutions) FIntPoint(5120, 2160); // UHD 5K
            new(StandardResolutions) FIntPoint(5120, 2880); // UHD+
            new(StandardResolutions) FIntPoint(15360, 8640); // QUHD
        }

        // Sort the list by total resolution size
        StandardResolutions.Sort
        ([](const FIntPoint& A, const FIntPoint& B)
        {
            return (A.X * A.Y) < (B.X * B.Y);
        });
    }

    // Return all standard resolutions that are within the size constraints
    for (const auto& Resolution : StandardResolutions)
    {
        if (Resolution.X >= MinResolution.X && Resolution.Y >= MinResolution.Y && Resolution.X <= MaxResolution.X && Resolution.Y <= MaxResolution.Y)
        {
            const float AspectRatio = Resolution.X / (float)Resolution.Y;
            if (AspectRatio > MinAspectRatio || FMath::IsNearlyEqual(AspectRatio, MinAspectRatio))
            {
                OutResolutions.Add(Resolution);
            }
        }
    }
}

FText UUR_SettingValueDiscrete_Resolution::FScreenResolutionEntry::GetDisplayText() const
{
    if (!OverrideText.IsEmpty())
    {
        return OverrideText;
    }

    FText Aspect = FText::GetEmpty();

    // expected: 4:3=1.333, 16:9=1.777, 16:10=1.6, multi-monitor-wide: >2
    float AspectRatio = (float)Width / (float)Height;

    if (FMath::Abs(AspectRatio - (4.0f / 3.0f)) < KINDA_SMALL_NUMBER)
    {
        Aspect = LOCTEXT("AspectRatio-4:3", "4:3");
    }
    else if (FMath::Abs(AspectRatio - (16.0f / 9.0f)) < KINDA_SMALL_NUMBER)
    {
        Aspect = LOCTEXT("AspectRatio-16:9", "16:9");
    }
    else if (FMath::Abs(AspectRatio - (16.0f / 10.0f)) < KINDA_SMALL_NUMBER)
    {
        Aspect = LOCTEXT("AspectRatio-16:10", "16:10");
    }
    else if (FMath::Abs(AspectRatio - (3.0f / 4.0f)) < KINDA_SMALL_NUMBER)
    {
        Aspect = LOCTEXT("AspectRatio-3:4", "3:4");
    }
    else if (FMath::Abs(AspectRatio - (9.0f / 16.0f)) < KINDA_SMALL_NUMBER)
    {
        Aspect = LOCTEXT("AspectRatio-9:16", "9:16");
    }
    else if (FMath::Abs(AspectRatio - (10.0f / 16.0f)) < KINDA_SMALL_NUMBER)
    {
        Aspect = LOCTEXT("AspectRatio-10:16", "10:16");
    }

    FNumberFormattingOptions Options;
    Options.UseGrouping = false;

    FFormatNamedArguments Args;
    Args.Add(TEXT("X"), FText::AsNumber(Width, &Options));
    Args.Add(TEXT("Y"), FText::AsNumber(Height, &Options));
    Args.Add(TEXT("AspectRatio"), Aspect);
    Args.Add(TEXT("RefreshRate"), RefreshRate);

    return FText::Format(LOCTEXT("AspectRatio", "{X} x {Y}"), Args);
}

#undef LOCTEXT_NAMESPACE
