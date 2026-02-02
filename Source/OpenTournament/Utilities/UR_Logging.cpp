// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_Logging.h"

#include "Engine/Engine.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

void URLogging::Log(FString inText, bool inLogToConsole, bool inLogToScreen, FColor inColor, float inTimeToDisplay, int32 inKey)
{
    //LogInternal(TEXT("%s"), inLogToConsole, inLogToScreen, inColor, inTimeToDisplay, inKey, *inText);
}

void URLogging::LogToConsoleInternal(FString inText)
{
    UE_LOG(LogTemp, Warning, TEXT("%s"), *inText);
}

void URLogging::LogToScreenInternal(FString inText, FColor inColor, float inTimeToDisplay, int32 inKey)
{
    if (GEngine != nullptr)
    {
        GEngine->AddOnScreenDebugMessage(inKey, inTimeToDisplay, inColor, inText);
    }
}
