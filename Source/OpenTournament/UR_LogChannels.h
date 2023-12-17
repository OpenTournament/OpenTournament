// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Logging/LogMacros.h"

class UObject;

/////////////////////////////////////////////////////////////////////////////////////////////////

DECLARE_LOG_CATEGORY_EXTERN(LogGame, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogGameUI, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogNetOT, Log, All);

OPENTOURNAMENT_API FString GetClientServerContextString(UObject* ContextObject = nullptr);
