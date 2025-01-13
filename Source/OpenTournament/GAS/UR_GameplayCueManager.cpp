// Copyright (c) Open Tournament Project, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_GameplayCueManager.h"

#include "AbilitySystemGlobals.h"
#include "GameplayCueSet.h"
#include "GameplayTagsManager.h"
#include "Async/Async.h"
#include "Engine/AssetManager.h"
#include "UObject/UObjectThreadContext.h"

#include "UR_LogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_GameplayCueManager)

/////////////////////////////////////////////////////////////////////////////////////////////////

enum class EGameEditorLoadMode
{
    // Loads all cues upfront; longer loading speed in the editor but short PIE times and effects never fail to play
    LoadUpfront,

    // Outside of editor: Async loads as cue tag are registered
    // In editor: Async loads when cues are invoked
    //   Note: This can cause some 'why didn't I see the effect for X' issues in PIE and is good for iteration speed but otherwise bad for designers
    PreloadAsCuesAreReferenced_GameOnly,

    // Async loads as cue tag are registered
    PreloadAsCuesAreReferenced
};

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace UR_GameplayCueManagerCvars
{
    static FAutoConsoleCommand CVarDumpGameplayCues(
        TEXT("OT.DumpGameplayCues"),
        TEXT("Shows all assets that were loaded via UR_GameplayCueManager and are currently in memory."),
        FConsoleCommandWithArgsDelegate::CreateStatic(UUR_GameplayCueManager::DumpGameplayCues));

    static EGameEditorLoadMode LoadMode = EGameEditorLoadMode::LoadUpfront;
}

const bool bPreloadEvenInEditor = true;

//////////////////////////////////////////////////////////////////////

struct FGameplayCueTagThreadSynchronizeGraphTask : public FAsyncGraphTaskBase
{
    TFunction<void()> TheTask;

    FGameplayCueTagThreadSynchronizeGraphTask(TFunction<void()>&& Task) :
        TheTask(MoveTemp(Task))
    {
    }

    void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
    {
        TheTask();
    }

    ENamedThreads::Type GetDesiredThread()
    {
        return ENamedThreads::GameThread;
    }
};

//////////////////////////////////////////////////////////////////////

UUR_GameplayCueManager::UUR_GameplayCueManager(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

UUR_GameplayCueManager* UUR_GameplayCueManager::Get()
{
    return Cast<UUR_GameplayCueManager>(UAbilitySystemGlobals::Get().GetGameplayCueManager());
}

void UUR_GameplayCueManager::OnCreated()
{
    Super::OnCreated();

    UpdateDelayLoadDelegateListeners();
}

void UUR_GameplayCueManager::LoadAlwaysLoadedCues()
{
    if (ShouldDelayLoadGameplayCues())
    {
        const UGameplayTagsManager& TagManager = UGameplayTagsManager::Get();

        //@TODO: Try to collect these by filtering GameplayCue. tags out of native gameplay tags?
        TArray<FName> AdditionalAlwaysLoadedCueTags;

        for (const FName& CueTagName : AdditionalAlwaysLoadedCueTags)
        {
            FGameplayTag CueTag = TagManager.RequestGameplayTag(CueTagName, /*ErrorIfNotFound=*/ false);
            if (CueTag.IsValid())
            {
                ProcessTagToPreload(CueTag, nullptr);
            }
            else
            {
                UE_LOG(LogGame, Warning, TEXT("UUR_GameplayCueManager::AdditionalAlwaysLoadedCueTags contains invalid tag %s"), *CueTagName.ToString());
            }
        }
    }
}

bool UUR_GameplayCueManager::ShouldAsyncLoadRuntimeObjectLibraries() const
{
    switch (UR_GameplayCueManagerCvars::LoadMode)
    {
        case EGameEditorLoadMode::LoadUpfront:
            {
                return true;
            }
        case EGameEditorLoadMode::PreloadAsCuesAreReferenced_GameOnly:
            {
#if WITH_EDITOR
                if (GIsEditor)
                {
                    return false;
                }
#endif
                break;
            }
        case EGameEditorLoadMode::PreloadAsCuesAreReferenced:
            {
                break;
            }
    }

    return !ShouldDelayLoadGameplayCues();
}

bool UUR_GameplayCueManager::ShouldSyncLoadMissingGameplayCues() const
{
    return false;
}

bool UUR_GameplayCueManager::ShouldAsyncLoadMissingGameplayCues() const
{
    return true;
}

void UUR_GameplayCueManager::DumpGameplayCues(const TArray<FString>& Args)
{
    UUR_GameplayCueManager* GCM = Cast<UUR_GameplayCueManager>(UAbilitySystemGlobals::Get().GetGameplayCueManager());
    if (!GCM)
    {
        UE_LOG(LogGame, Error, TEXT("DumpGameplayCues failed. No UR_GameplayCueManager found."));
        return;
    }

    const bool bIncludeRefs = Args.Contains(TEXT("Refs"));

    UE_LOG(LogGame, Log, TEXT("=========== Dumping Always Loaded Gameplay Cue Notifies ==========="));
    for (UClass* CueClass : GCM->AlwaysLoadedCues)
    {
        UE_LOG(LogGame, Log, TEXT("  %s"), *GetPathNameSafe(CueClass));
    }

    UE_LOG(LogGame, Log, TEXT("=========== Dumping Preloaded Gameplay Cue Notifies ==========="));
    for (UClass* CueClass : GCM->PreloadedCues)
    {
        TSet<FObjectKey>* ReferencerSet = GCM->PreloadedCueReferencers.Find(CueClass);
        int32 NumRefs = ReferencerSet ? ReferencerSet->Num() : 0;
        UE_LOG(LogGame, Log, TEXT("  %s (%d refs)"), *GetPathNameSafe(CueClass), NumRefs);
        if (bIncludeRefs && ReferencerSet)
        {
            for (const FObjectKey& Ref : *ReferencerSet)
            {
                UObject* RefObject = Ref.ResolveObjectPtr();
                UE_LOG(LogGame, Log, TEXT("    ^- %s"), *GetPathNameSafe(RefObject));
            }
        }
    }

    UE_LOG(LogGame, Log, TEXT("=========== Dumping Gameplay Cue Notifies loaded on demand ==========="));
    int32 NumMissingCuesLoaded = 0;
    if (GCM->RuntimeGameplayCueObjectLibrary.CueSet)
    {
        for (const FGameplayCueNotifyData& CueData : GCM->RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueData)
        {
            if (CueData.LoadedGameplayCueClass && !GCM->AlwaysLoadedCues.Contains(CueData.LoadedGameplayCueClass) && !GCM->PreloadedCues.Contains(CueData.LoadedGameplayCueClass))
            {
                NumMissingCuesLoaded++;
                UE_LOG(LogGame, Log, TEXT("  %s"), *CueData.LoadedGameplayCueClass->GetPathName());
            }
        }
    }

    UE_LOG(LogGame, Log, TEXT("=========== Gameplay Cue Notify summary ==========="));
    UE_LOG(LogGame, Log, TEXT("  ... %d cues in always loaded list"), GCM->AlwaysLoadedCues.Num());
    UE_LOG(LogGame, Log, TEXT("  ... %d cues in preloaded list"), GCM->PreloadedCues.Num());
    UE_LOG(LogGame, Log, TEXT("  ... %d cues loaded on demand"), NumMissingCuesLoaded);
    UE_LOG(LogGame, Log, TEXT("  ... %d cues in total"), GCM->AlwaysLoadedCues.Num() + GCM->PreloadedCues.Num() + NumMissingCuesLoaded);
}

void UUR_GameplayCueManager::OnGameplayTagLoaded(const FGameplayTag& Tag)
{
    FScopeLock ScopeLock(&LoadedGameplayTagsToProcessCS);
    bool bStartTask = LoadedGameplayTagsToProcess.Num() == 0;
    FUObjectSerializeContext* LoadContext = FUObjectThreadContext::Get().GetSerializeContext();
    UObject* OwningObject = LoadContext ? LoadContext->SerializedObject : nullptr;
    LoadedGameplayTagsToProcess.Emplace(Tag, OwningObject);
    if (bStartTask)
    {
        TGraphTask<FGameplayCueTagThreadSynchronizeGraphTask>::CreateTask().ConstructAndDispatchWhenReady([]()
        {
            if (GIsRunning)
            {
                if (UUR_GameplayCueManager* StrongThis = Get())
                {
                    // If we are garbage collecting we cannot call StaticFindObject (or a few other static uobject functions), so we'll just wait until the GC is over and process the tags then
                    if (IsGarbageCollecting())
                    {
                        StrongThis->bProcessLoadedTagsAfterGC = true;
                    }
                    else
                    {
                        StrongThis->ProcessLoadedTags();
                    }
                }
            }
        });
    }
}

void UUR_GameplayCueManager::HandlePostGarbageCollect()
{
    if (bProcessLoadedTagsAfterGC)
    {
        ProcessLoadedTags();
    }
    bProcessLoadedTagsAfterGC = false;
}

void UUR_GameplayCueManager::ProcessLoadedTags()
{
    TArray<FLoadedGameplayTagToProcessData> TaskLoadedGameplayTagsToProcess;
    {
        // Lock LoadedGameplayTagsToProcess just long enough to make a copy and clear
        FScopeLock TaskScopeLock(&LoadedGameplayTagsToProcessCS);
        TaskLoadedGameplayTagsToProcess = LoadedGameplayTagsToProcess;
        LoadedGameplayTagsToProcess.Empty();
    }

    // This might return during shutdown, and we don't want to proceed if that is the case
    if (GIsRunning)
    {
        if (RuntimeGameplayCueObjectLibrary.CueSet)
        {
            for (const FLoadedGameplayTagToProcessData& LoadedTagData : TaskLoadedGameplayTagsToProcess)
            {
                if (RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueDataMap.Contains(LoadedTagData.Tag))
                {
                    if (!LoadedTagData.WeakOwner.IsStale())
                    {
                        ProcessTagToPreload(LoadedTagData.Tag, LoadedTagData.WeakOwner.Get());
                    }
                }
            }
        }
        else
        {
            UE_LOG(LogGame, Warning, TEXT("UR_GameplayCueManager::OnGameplayTagLoaded processed loaded tag(s) but RuntimeGameplayCueObjectLibrary.CueSet was null. Skipping processing."));
        }
    }
}

void UUR_GameplayCueManager::ProcessTagToPreload(const FGameplayTag& Tag, UObject* OwningObject)
{
    switch (UR_GameplayCueManagerCvars::LoadMode)
    {
        case EGameEditorLoadMode::LoadUpfront:
            {
                return;
            }
        case EGameEditorLoadMode::PreloadAsCuesAreReferenced_GameOnly:
            {
#if WITH_EDITOR
                if (GIsEditor)
                {
                    return;
                }
#endif
                break;
            }
        case EGameEditorLoadMode::PreloadAsCuesAreReferenced:
            {
                break;
            }
    }

    check(RuntimeGameplayCueObjectLibrary.CueSet);

    int32* DataIdx = RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueDataMap.Find(Tag);
    if (DataIdx && RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueData.IsValidIndex(*DataIdx))
    {
        const FGameplayCueNotifyData& CueData = RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueData[*DataIdx];

        UClass* LoadedGameplayCueClass = FindObject<UClass>(nullptr, *CueData.GameplayCueNotifyObj.ToString());
        if (LoadedGameplayCueClass)
        {
            RegisterPreloadedCue(LoadedGameplayCueClass, OwningObject);
        }
        else
        {
            bool bAlwaysLoadedCue = OwningObject == nullptr;
            TWeakObjectPtr<UObject> WeakOwner = OwningObject;
            StreamableManager.RequestAsyncLoad(CueData.GameplayCueNotifyObj,
                FStreamableDelegate::CreateUObject(this, &ThisClass::OnPreloadCueComplete, CueData.GameplayCueNotifyObj, WeakOwner, bAlwaysLoadedCue),
                FStreamableManager::DefaultAsyncLoadPriority,
                false,
                false,
                TEXT("GameplayCueManager"));
        }
    }
}

void UUR_GameplayCueManager::OnPreloadCueComplete(FSoftObjectPath Path, TWeakObjectPtr<UObject> OwningObject, bool bAlwaysLoadedCue)
{
    if (bAlwaysLoadedCue || OwningObject.IsValid())
    {
        if (UClass* LoadedGameplayCueClass = Cast<UClass>(Path.ResolveObject()))
        {
            RegisterPreloadedCue(LoadedGameplayCueClass, OwningObject.Get());
        }
    }
}

void UUR_GameplayCueManager::RegisterPreloadedCue(UClass* LoadedGameplayCueClass, UObject* OwningObject)
{
    check(LoadedGameplayCueClass);

    const bool bAlwaysLoadedCue = OwningObject == nullptr;
    if (bAlwaysLoadedCue)
    {
        AlwaysLoadedCues.Add(LoadedGameplayCueClass);
        PreloadedCues.Remove(LoadedGameplayCueClass);
        PreloadedCueReferencers.Remove(LoadedGameplayCueClass);
    }
    else if ((OwningObject != LoadedGameplayCueClass) && (OwningObject != LoadedGameplayCueClass->GetDefaultObject()) && !AlwaysLoadedCues.Contains(LoadedGameplayCueClass))
    {
        PreloadedCues.Add(LoadedGameplayCueClass);
        TSet<FObjectKey>& ReferencerSet = PreloadedCueReferencers.FindOrAdd(LoadedGameplayCueClass);
        ReferencerSet.Add(OwningObject);
    }
}

void UUR_GameplayCueManager::HandlePostLoadMap(UWorld* NewWorld)
{
    if (RuntimeGameplayCueObjectLibrary.CueSet)
    {
        for (UClass* CueClass : AlwaysLoadedCues)
        {
            RuntimeGameplayCueObjectLibrary.CueSet->RemoveLoadedClass(CueClass);
        }

        for (UClass* CueClass : PreloadedCues)
        {
            RuntimeGameplayCueObjectLibrary.CueSet->RemoveLoadedClass(CueClass);
        }
    }

    for (auto CueIt = PreloadedCues.CreateIterator(); CueIt; ++CueIt)
    {
        TSet<FObjectKey>& ReferencerSet = PreloadedCueReferencers.FindChecked(*CueIt);
        for (auto RefIt = ReferencerSet.CreateIterator(); RefIt; ++RefIt)
        {
            if (!RefIt->ResolveObjectPtr())
            {
                RefIt.RemoveCurrent();
            }
        }
        if (ReferencerSet.Num() == 0)
        {
            PreloadedCueReferencers.Remove(*CueIt);
            CueIt.RemoveCurrent();
        }
    }
}

void UUR_GameplayCueManager::UpdateDelayLoadDelegateListeners()
{
    UGameplayTagsManager::Get().OnGameplayTagLoadedDelegate.RemoveAll(this);
    FCoreUObjectDelegates::GetPostGarbageCollect().RemoveAll(this);
    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

    switch (UR_GameplayCueManagerCvars::LoadMode)
    {
        case EGameEditorLoadMode::LoadUpfront:
            return;
        case EGameEditorLoadMode::PreloadAsCuesAreReferenced_GameOnly:
#if WITH_EDITOR
            if (GIsEditor)
            {
                return;
            }
#endif
            break;
        case EGameEditorLoadMode::PreloadAsCuesAreReferenced:
            break;
    }

    UGameplayTagsManager::Get().OnGameplayTagLoadedDelegate.AddUObject(this, &ThisClass::OnGameplayTagLoaded);
    FCoreUObjectDelegates::GetPostGarbageCollect().AddUObject(this, &ThisClass::HandlePostGarbageCollect);
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::HandlePostLoadMap);
}

bool UUR_GameplayCueManager::ShouldDelayLoadGameplayCues() const
{
    const bool bClientDelayLoadGameplayCues = true;
    return !IsRunningDedicatedServer() && bClientDelayLoadGameplayCues;
}

const FPrimaryAssetType UFortAssetManager_GameplayCueRefsType = TEXT("GameplayCueRefs");
const FName UFortAssetManager_GameplayCueRefsName = TEXT("GameplayCueReferences");
const FName UFortAssetManager_LoadStateClient = FName(TEXT("Client"));

void UUR_GameplayCueManager::RefreshGameplayCuePrimaryAsset()
{
    TArray<FSoftObjectPath> CuePaths;
    UGameplayCueSet* RuntimeGameplayCueSet = GetRuntimeCueSet();
    if (RuntimeGameplayCueSet)
    {
        RuntimeGameplayCueSet->GetSoftObjectPaths(CuePaths);
    }

    FAssetBundleData BundleData;
    BundleData.AddBundleAssetsTruncated(UFortAssetManager_LoadStateClient, CuePaths);

    FPrimaryAssetId PrimaryAssetId = FPrimaryAssetId(UFortAssetManager_GameplayCueRefsType, UFortAssetManager_GameplayCueRefsName);
    UAssetManager::Get().AddDynamicAsset(PrimaryAssetId, FSoftObjectPath(), BundleData);
}
