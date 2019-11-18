// Copyright 2019 Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_FunctionLibrary.h"

#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"

#include "UR_GameModeBase.h"
#include "UR_PlayerController.h"
#include "UR_PlayerState.h"

#include "UnrealString.h"
#include "Regex.h"

#include "GameFramework/InputSettings.h"

#include "Engine.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

AUR_GameModeBase* UUR_FunctionLibrary::GetGameModeDefaultObject(const UObject* WorldContextObject)
{
    AUR_GameModeBase* GameModeDefaultObject = nullptr;
    
    if (const auto World = WorldContextObject->GetWorld())
    {
        if (const auto GameState = World->GetGameState())
        {
            if (auto GameModeClass = GameState->GameModeClass)
            {
                GameModeDefaultObject = Cast<AUR_GameModeBase>(GameModeClass->GetDefaultObject());
            }
        }
    }

    return GameModeDefaultObject;
}

int32 UUR_FunctionLibrary::GetPlayerStateValue(APlayerController* PlayerController)
{
    int32 outValue = -1;
    if (PlayerController)
    {
        if (const auto PlayerState = Cast<AUR_PlayerState>(PlayerController->PlayerState))
        {
            outValue = PlayerState->ArbitraryValue;
        }
    }
    return outValue;
}

FColor UUR_FunctionLibrary::GetPlayerDisplayTextColor(APlayerState* PS)
{
    if (!PS)
    {
        return FColorList::Green;
    }
    else if (PS->bOnlySpectator)
    {
        return GetSpectatorDisplayTextColor();
    }
    else
    {
        AUR_PlayerState* URPS = Cast<AUR_PlayerState>(PS);
        if (URPS)
        {
            //TODO: if team game, return team color, something like URPS->Team->GetDisplayTextColor();

            //TODO: if non team game, return player's color ? if any ? or white ?

            return FColorList::Red;
        }
        else
        {
            return FColorList::Green;	//???
        }
    }
}

FString UUR_FunctionLibrary::StripRichTextDecorators(const FString& InText)
{
    FString Result(TEXT(""), InText.Len());

    int32 CurrentPosition = 0;

    // see RichTextMarkupProcessing.cpp

    TArray<FTextRange> LineRanges;
    FTextRange::CalculateLineRangesFromString(InText, LineRanges);

    FRegexPattern ElementRegexPattern(TEXT("<([\\w\\d\\.-]+)((?: (?:[\\w\\d\\.-]+=(?>\".*?\")))+)?(?:(?:/>)|(?:>(.*?)</>))"));
    FRegexMatcher ElementRegexMatcher(ElementRegexPattern, InText);

    for (int32 i = 0; i < LineRanges.Num(); i++)
    {
        // Limit the element regex matcher to the current line
        ElementRegexMatcher.SetLimits(LineRanges[i].BeginIndex, LineRanges[i].EndIndex);

        while (ElementRegexMatcher.FindNext())
        {
            // append all from current position up to opening marker
            Result.Append(InText.Mid(CurrentPosition, ElementRegexMatcher.GetMatchBeginning() - CurrentPosition));
            // append inner content
            Result.Append(ElementRegexMatcher.GetCaptureGroup(3));
            // set current position to after closing marker
            CurrentPosition = ElementRegexMatcher.GetMatchEnding();
        }
    }

    Result.Append(InText.Mid(CurrentPosition));

    return Result;
}


bool UUR_FunctionLibrary::IsKeyMappedToAction(const FKey& Key, FName ActionName)
{
    UInputSettings* Settings = UInputSettings::GetInputSettings();
    TArray<FInputActionKeyMapping> Mappings;
    Settings->GetActionMappingByName(ActionName, Mappings);
    for (const auto& Mapping : Mappings)
    {
        if (Mapping.Key == Key)
            return true;
    }
    return false;
}

bool UUR_FunctionLibrary::IsKeyMappedToAxis(const FKey& Key, FName AxisName, float Direction)
{
    UInputSettings* Settings = UInputSettings::GetInputSettings();
    TArray<FInputAxisKeyMapping> Mappings;
    Settings->GetAxisMappingByName(AxisName, Mappings);
    for (const auto& Mapping : Mappings)
    {
        if (Mapping.Key == Key && (FMath::IsNearlyZero(Direction) || (Direction*Mapping.Scale > 0)))
            return true;
    }
    return false;
}

AUR_PlayerController* UUR_FunctionLibrary::GetLocalPlayerController(const UObject* WorldContextObject)
{
    if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
        return World->GetFirstPlayerController<AUR_PlayerController>();

    return nullptr;
}

FString UUR_FunctionLibrary::GetTimeString(const float TimeSeconds)
{
    // only minutes and seconds are relevant
    const int32 TotalSeconds = FMath::Max(0, FMath::TruncToInt(TimeSeconds) % 3600);
    const int32 NumMinutes = TotalSeconds / 60;
    const int32 NumSeconds = TotalSeconds % 60;

    const FString TimeDesc = FString::Printf(TEXT("%02d:%02d"), NumMinutes, NumSeconds);
    return TimeDesc;
}