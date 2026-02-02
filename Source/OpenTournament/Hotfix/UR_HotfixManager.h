// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "OnlineHotfixManager.h"

#include "Misc/CoreDelegates.h"
#include "Containers/Ticker.h"

#include "UR_HotfixManager.generated.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class UUR_HotfixManager : public UOnlineHotfixManager
{
    GENERATED_BODY()

public:
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnPendingGameHotfix, bool);
    FOnPendingGameHotfix OnPendingGameHotfixChanged;

    UUR_HotfixManager();
    virtual ~UUR_HotfixManager() override;

    void RequestPatchAssetsFromIniFiles();

protected:
    void OnHotfixCompleted(EHotfixResult HotfixResult);

    virtual FString GetCachedDirectory() override
    {
        return FPaths::ProjectPersistentDownloadDir() / TEXT("Hotfix/");
    }

    virtual void StartHotfixProcess() override;

    virtual bool WantsHotfixProcessing(const struct FCloudFileHeader& FileHeader) override;
    virtual bool ApplyHotfixProcessing(const struct FCloudFileHeader& FileHeader) override;
    virtual bool ShouldWarnAboutMissingWhenPatchingFromIni(const FString& AssetPath) const override;
    virtual void PatchAssetsFromIniFiles() override;

    virtual void OnHotfixAvailablityCheck(const TArray<FCloudFileHeader>& PendingChangedFiles, const TArray<FCloudFileHeader>& PendingRemoveFiles) override;

    virtual bool HotfixIniFile(const FString& FileName, const FString& IniData) override;

private:
#if !UE_BUILD_SHIPPING
    // Error reporting
    FDelegateHandle OnScreenMessageHandle;
    void GetOnScreenMessages(TMultiMap<FCoreDelegates::EOnScreenMessageSeverity, FText>& OutMessages);
#endif // !UE_BUILD_SHIPPING

private:
    /** get the current game instance */
    template <typename T>
    T* GetGameInstance() const
    {
        return GetTypedOuter<T>();
    }

    virtual void Init() override;

private:
    FTSTicker::FDelegateHandle RequestPatchAssetsHandle;
    FDelegateHandle HotfixCompleteDelegateHandle;

    bool bHasPendingGameHotfix = false;
    bool bHasPendingDeviceProfileHotfix = false;

    static int32 GameHotfixCounter;
};
