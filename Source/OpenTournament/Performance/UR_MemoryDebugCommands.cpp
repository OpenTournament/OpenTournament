// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "Engine/BlueprintGeneratedClass.h"
#include "UObject/UObjectIterator.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Engine/World.h"

#include "UR_LogChannels.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

#if ALLOW_DEBUG_FILES
#include "HAL/IConsoleManager.h"

// Writes a collection of the specified name containing a list of items (returning the absolute file path to the collection)
// This is manual rather than relying on the collection manager so it can be used at runtime without depending on a developer module
FString WriteCollectionFile(const FString& CollectionName, const TArray<FString>& Items)
{
    // If in the editor, create it in the directory that CST_Local would have used, otherwise write it to the profiling dir for later harvesting
    const FString OutputDir = WITH_EDITOR ? (FPaths::ProjectSavedDir() / TEXT("Collections")) : (FPaths::ProfilingDir() / TEXT("AssetSnapshots"));

    IFileManager::Get().MakeDirectory(*OutputDir, true);

    const FString LogFilename = OutputDir / (CollectionName + TEXT(".collection"));

    if (FArchive* OutputFile = IFileManager::Get().CreateDebugFileWriter(*LogFilename))
    {
        const FGuid CollectionGUID = FGuid::NewGuid();

        OutputFile->Logf(TEXT("FileVersion:2"));
        OutputFile->Logf(TEXT("Type:Static"));
        OutputFile->Logf(TEXT("Guid:%s"), *CollectionGUID.ToString(EGuidFormats::DigitsWithHyphens));
        OutputFile->Logf(TEXT(""));

        for (const FString& Item : Items)
        {
            OutputFile->Logf(TEXT("%s"), *Item);
        }

        // Flush, close and delete.
        delete OutputFile;

        const FString AbsolutePath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*LogFilename);
        return AbsolutePath;
    }

    return FString();
}

FAutoConsoleCommandWithWorldAndArgs GObjListToCollectionCmd
(
    TEXT("OT.ObjListToCollection"),
    TEXT("Spits out a collection that contains the current object list"),
    FConsoleCommandWithWorldAndArgsDelegate::CreateStatic
    (
        [](const TArray<FString>& Params, UWorld* World)
        {
            // Get the list of loaded assets
            TArray<FString> AssetPaths;
            for (TObjectIterator<UObject> It; It; ++It)
            {
                UObject* Obj = *It;
                if (Obj->IsAsset())
                {
                    AssetPaths.Add(Obj->GetPathName());
                }
                else if (UBlueprintGeneratedClass* Class = Cast<UBlueprintGeneratedClass>(Obj))
                {
                    FString BlueprintName = Class->GetPathName();
                    BlueprintName.RemoveFromEnd(TEXT("_C"));
                    AssetPaths.Add(BlueprintName);
                }
            }
            AssetPaths.Sort();

            // Determine the filename
            FString CollectionNameSuffix;
            if (Params.Num() > 0)
            {
                CollectionNameSuffix = TEXT("_") + Params[0];
            }
            const FString CollectionName = FString::Printf(TEXT("_LoadedAssets_%s_%s%s"), *FDateTime::Now().ToString(TEXT("%H%M%S")), *GWorld->GetMapName(), *CollectionNameSuffix);

            // Write the collection out
            const FString CollectionFilePath = WriteCollectionFile(CollectionName, AssetPaths);
            UE_LOG(LogGame, Warning, TEXT("Wrote collection of loaded assets to %s"), *CollectionFilePath);
        }));

#endif

//////////////////////////////////////////////////////////////////////////

// This can be used in a command to compare assets to a parent class (BP or C++ default) to determine if any fields are actually 'fixed' and can be removed to save memory
void AnalyzeObjectListForDifferences(TArrayView<UObject*> ObjectList, UClass* CommonClass, const TSet<FName>& PropertiesToIgnore, bool bLogAllMatchedDefault = false)
{
    check(CommonClass);
    UObject* CommonClassCDO = CommonClass->GetDefaultObject();

    UE_LOG(LogGame, Log, TEXT("  Field\tDifferentToBase\tNumValues\tValues"));

    for (TFieldIterator<FProperty> PropIt(CommonClass); PropIt; ++PropIt)
    {
        FProperty* Prop = *PropIt;
        if (PropertiesToIgnore.Contains(Prop->GetFName()))
        {
            continue;
        }

        //@TODO: Handle fixed length arrays
        ensure(Prop->ArrayDim <= 1);

        FString DefaultValueStr;
        Prop->ExportText_InContainer(0, /*out*/ DefaultValueStr, CommonClassCDO, CommonClassCDO, nullptr, 0);

        bool bAnyMatchedDefaultValue = false;
        bool bAllMatchedDefaultValue = true;

        TSet<FString> ValuesObserved;
        for (UObject* Object : ObjectList)
        {
            FString ValueStr;
            if (Prop->ExportText_InContainer(0, /*out*/ ValueStr, Object, CommonClassCDO, nullptr, 0))
            {
                ValuesObserved.Add(ValueStr);
                bAllMatchedDefaultValue = false;
            }
            else
            {
                bAnyMatchedDefaultValue = true;
            }
        }

        if (bAnyMatchedDefaultValue)
        {
            ValuesObserved.Add(DefaultValueStr);
        }

        if (!bAllMatchedDefaultValue)
        {
            const FString ValueList = FString::Join(ValuesObserved, TEXT(","));

            UE_LOG
            (LogGame,
                Log,
                TEXT("  %s::%s\t%s\t%d\t%s"),
                *CommonClass->GetName(),
                *Prop->GetName(),
                (ValuesObserved.Num() == 1) ? TEXT("FixedDifferent") : TEXT("Varies"),
                ValuesObserved.Num(),
                *ValueList);
        }
        else if (bLogAllMatchedDefault)
        {
            UE_LOG(LogGame, Log, TEXT("  %s::%s\t%s"), *CommonClass->GetName(), *Prop->GetName(), TEXT("Default"));
        }
    }
}

//////////////////////////////////////////////////////////////////////////
