// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_HotfixManager.h"

#include "DeviceProfiles/DeviceProfile.h"
#include "DeviceProfiles/DeviceProfileManager.h"
#include "Engine/NetDriver.h"
#include "HAL/MemoryMisc.h"
#include "UObject/UObjectIterator.h"

#include "Settings/UR_SettingsLocal.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_HotfixManager)

/////////////////////////////////////////////////////////////////////////////////////////////////

int32 UUR_HotfixManager::GameHotfixCounter = 0;

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_HotfixManager::UUR_HotfixManager()
{
#if !UE_BUILD_SHIPPING
    OnScreenMessageHandle = FCoreDelegates::OnGetOnScreenMessages.AddUObject(this, &UUR_HotfixManager::GetOnScreenMessages);
#endif // !UE_BUILD_SHIPPING

    HotfixCompleteDelegateHandle = AddOnHotfixCompleteDelegate_Handle(FOnHotfixCompleteDelegate::CreateUObject(this, &ThisClass::OnHotfixCompleted));
}

void UUR_HotfixManager::Init()
{
    Super::Init();
}

void UUR_HotfixManager::OnHotfixCompleted(EHotfixResult HotfixResult)
{
    // Reload DDoS detection config for all live Net Drivers (mirrors RepGraph code)
    for (TObjectIterator<UNetDriver> It; It; ++It)
    {
        if (It->IsServer())
        {
            UE_LOG(LogHotfixManager, Log, TEXT("Reloading DDoS detection settings for NetDriver: %s"), *It->GetName());

            It->DDoS.InitConfig();
        }
    }

    if (bHasPendingDeviceProfileHotfix)
    {
        UE_LOG(LogHotfixManager, Log, TEXT("Re-applying Hotfixed DeviceProfile"));

        bHasPendingDeviceProfileHotfix = false;
        UDeviceProfileManager::Get().ReapplyDeviceProfile();

        UUR_SettingsLocal* GameSettings = UUR_SettingsLocal::Get();
        GameSettings->OnHotfixDeviceProfileApplied();
    }

#if ENABLE_SHARED_MEMORY_TRACKER
    FSharedMemoryTracker::PrintMemoryDiff(TEXT("Hotfix Complete"));
#endif
}

UUR_HotfixManager::~UUR_HotfixManager()
{
    ClearOnHotfixCompleteDelegate_Handle(HotfixCompleteDelegateHandle);

#if !UE_BUILD_SHIPPING
    FCoreDelegates::OnGetOnScreenMessages.Remove(OnScreenMessageHandle);
#endif // !UE_BUILD_SHIPPING
}

bool UUR_HotfixManager::WantsHotfixProcessing(const FCloudFileHeader& FileHeader)
{
    bool bWantsProcessing = Super::WantsHotfixProcessing(FileHeader);
    if (!bWantsProcessing)
    {
        FString SupportedFiles[] = {
            TEXT("AssetMigrations.ini")
        };

        for (FString SupportedFile : SupportedFiles)
        {
#if !UE_BUILD_SHIPPING
            if (!DebugPrefix.IsEmpty())
            {
                SupportedFile = DebugPrefix + SupportedFile;
            }
#endif
            if (SupportedFile == FileHeader.FileName)
            {
                bWantsProcessing = true;
                break;
            }
        }
    }
    return bWantsProcessing;
}

bool UUR_HotfixManager::HotfixIniFile(const FString& FileName, const FString& IniData)
{
    if (!bHasPendingDeviceProfileHotfix && FileName.EndsWith(TEXT("DEVICEPROFILES.INI"), ESearchCase::IgnoreCase))
    {
        FConfigFile DeviceProfileHotfixConfig;
        DeviceProfileHotfixConfig.CombineFromBuffer(IniData, FileName);
        TSet<FString> Keys;
        for (const auto& DeviceProfileSection : AsConst(DeviceProfileHotfixConfig))
        {
            FString DeviceProfileName, DeviceProfileClass;
            if (DeviceProfileSection.Key.Split(TEXT(" "), &DeviceProfileName, &DeviceProfileClass) && DeviceProfileClass == *UDeviceProfile::StaticClass()->GetName())
            {
                Keys.Add(DeviceProfileName);
            }
        }

        // Check if any of the hotfixed device profiles are referenced by the currently active profile(s):
        bHasPendingDeviceProfileHotfix = UDeviceProfileManager::Get().DoActiveProfilesReference(Keys);
        UE_LOG(LogHotfixManager, Log, TEXT("Active device profile was referenced by hotfix = %d"), (uint32)bHasPendingDeviceProfileHotfix);
    }

    return Super::HotfixIniFile(FileName, IniData);
}

bool UUR_HotfixManager::ApplyHotfixProcessing(const FCloudFileHeader& FileHeader)
{
    // This allows json files to be downloaded automatically
    const FString Extension = FPaths::GetExtension(FileHeader.FileName);
    if (Extension == TEXT("json"))
    {
        return true;
    }

    const bool bResult = Super::ApplyHotfixProcessing(FileHeader);
    if (bResult && FileHeader.FileName.EndsWith(TEXT("GAME.INI"), ESearchCase::IgnoreCase))
    {
        GameHotfixCounter++;

        if (bHasPendingGameHotfix)
        {
            bHasPendingGameHotfix = false;
            OnPendingGameHotfixChanged.Broadcast(bHasPendingGameHotfix);
        }
    }

    return bResult;
}

bool UUR_HotfixManager::ShouldWarnAboutMissingWhenPatchingFromIni(const FString& AssetPath) const
{
    return AssetPath.StartsWith(TEXT("/Engine/")) || AssetPath.StartsWith(TEXT("/Game/"));
}

void UUR_HotfixManager::PatchAssetsFromIniFiles()
{
#if ENABLE_SHARED_MEMORY_TRACKER
    FSharedMemoryTracker::PrintMemoryDiff(TEXT("Start - PatchAssetsFromIniFiles"));
#endif

    Super::PatchAssetsFromIniFiles();

#if ENABLE_SHARED_MEMORY_TRACKER
    FSharedMemoryTracker::PrintMemoryDiff(TEXT("End - PatchAssetsFromIniFiles"));
#endif
}

void UUR_HotfixManager::OnHotfixAvailablityCheck(const TArray<FCloudFileHeader>& PendingChangedFiles, const TArray<FCloudFileHeader>& PendingRemoveFiles)
{
    bool bNewPendingGameHotfix = false;
    for (int32 Idx = 0; Idx < PendingChangedFiles.Num(); Idx++)
    {
        if (PendingChangedFiles[Idx].FileName.EndsWith(TEXT("GAME.INI"), ESearchCase::IgnoreCase))
        {
            bNewPendingGameHotfix = true;
            break;
        }
    }

    if (bNewPendingGameHotfix && !bHasPendingGameHotfix)
    {
        bHasPendingGameHotfix = true;
        OnPendingGameHotfixChanged.Broadcast(bHasPendingGameHotfix);
    }
}

#if !UE_BUILD_SHIPPING
void UUR_HotfixManager::GetOnScreenMessages(TMultiMap<FCoreDelegates::EOnScreenMessageSeverity, FText>& OutMessages)
{
    // TODO Any messages/errors.
}
#endif // !UE_BUILD_SHIPPING

void UUR_HotfixManager::RequestPatchAssetsFromIniFiles()
{
    if (!RequestPatchAssetsHandle.IsValid())
    {
        RequestPatchAssetsHandle = FTSTicker::GetCoreTicker().AddTicker
        (
            FTickerDelegate::CreateWeakLambda
            (this,
                [this](float DeltaTime)
                {
                    RequestPatchAssetsHandle.Reset();
                    UE_LOG(LogHotfixManager, Display, TEXT("Hotfix manager re-calling PatchAssetsFromIniFiles due to new plugins"));
                    PatchAssetsFromIniFiles();
                    return false;
                }
            )
        );
    }
}

void UUR_HotfixManager::StartHotfixProcess()
{
    if (GIsEditor)
    {
        UE_LOG(LogHotfixManager, Display, TEXT("Hotfixing skipped in development mode."));
        TriggerHotfixComplete(EHotfixResult::SuccessNoChange);
        return;
    }

#if ENABLE_SHARED_MEMORY_TRACKER
    FSharedMemoryTracker::PrintMemoryDiff(TEXT("StartHotfixProcess"));
#endif

    Super::StartHotfixProcess();
}
