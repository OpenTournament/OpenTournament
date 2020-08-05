// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

#include "UR_MPC_Global.h"

#include "Kismet/KismetMaterialLibrary.h"
#include "Engine/Engine.h"
#include "Materials/MaterialParameterCollection.h"

#include "UR_PlayerController.h"


void UUR_MPC_Global::SetScalar(UObject* WorldContext, FName Param, float Value)
{
    UKismetMaterialLibrary::SetScalarParameterValue(WorldContext, GetCollection(WorldContext), Param, Value);

    // Cascade update to mapped parameters
    TSet<FName> MappedParams;
    GetMappedParameters(Param, MappedParams);
    for (FName& Target : MappedParams)
    {
        SetScalar(WorldContext, Target, Value);
    }
}

float UUR_MPC_Global::GetScalar(UObject* WorldContext, FName Param)
{
    return UKismetMaterialLibrary::GetScalarParameterValue(WorldContext, GetCollection(WorldContext), Param);
}

void UUR_MPC_Global::SetVector(UObject* WorldContext, FName Param, const FLinearColor& Value)
{
    UKismetMaterialLibrary::SetVectorParameterValue(WorldContext, GetCollection(WorldContext), Param, Value);

    // Cascade update to mapped parameters
    TSet<FName> MappedParams;
    GetMappedParameters(Param, MappedParams);
    for (FName& Target : MappedParams)
    {
        SetVector(WorldContext, Target, Value);
    }
}

FLinearColor UUR_MPC_Global::GetVector(UObject* WorldContext, FName Param)
{
    return UKismetMaterialLibrary::GetVectorParameterValue(WorldContext, GetCollection(WorldContext), Param);
}


bool UUR_MPC_Global::MapParameter(UObject* WorldContext, FName TargetParam, FName SourceParam)
{
    UnmapParameter(WorldContext, TargetParam);

    if (SourceParam == NAME_None || TargetParam == NAME_None || SourceParam.IsEqual(TargetParam))
    {
        return false;
    }

    // Check circular references
    TSet<FName> CurrentSet = { TargetParam };
    for (int32 i = 0; (CurrentSet.Num() > 0) && (i < 100); i++)
    {
        TSet<FName> NewSet;
        for (FName& Target : CurrentSet)
        {
            GetMappedParameters(Target, NewSet);
        }
        if (NewSet.Contains(SourceParam))
        {
            UE_LOG(LogTemp, Warning, TEXT("MPCGlobal prevented mapping '%s' to '%s' due to circular reference"), *SourceParam.ToString(), *TargetParam.ToString());
            return false;
        }
        CurrentSet = NewSet;
    }
    if (CurrentSet.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("MPCGlobal detected existing circular reference! Calls to SetScalar/SetVector might cause crash!"));
    }

    // Check parameter compatibility & create mapping
    const auto Collection = GetCollection(WorldContext);
    auto Params = GetMutableDefault<UUR_MPC_Global>();
    if (Collection && Params)
    {
        if (Collection->GetScalarParameterByName(SourceParam))
        {
            if (Collection->GetScalarParameterByName(TargetParam))
            {
                Params->Mappings.Add(TargetParam, SourceParam);
                SetScalar(WorldContext, TargetParam, GetScalar(WorldContext, SourceParam));
                return true;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("MPCGlobal parameter '%s' is not compatible with '%s'"), *SourceParam.ToString(), *TargetParam.ToString());
            }
        }
        else if (Collection->GetVectorParameterByName(SourceParam))
        {
            if (Collection->GetVectorParameterByName(TargetParam))
            {
                Params->Mappings.Add(TargetParam, SourceParam);
                SetVector(WorldContext, TargetParam, GetVector(WorldContext, SourceParam));
                return true;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("MPCGlobal parameter '%s' is not compatible with '%s'"), *SourceParam.ToString(), *TargetParam.ToString());
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("MPCGlobal parameter '%s' not found"), *SourceParam.ToString());
        }
    }

    return false;
}

void UUR_MPC_Global::UnmapParameter(UObject* WorldContext, FName TargetParam)
{
    if (auto Params = GetMutableDefault<UUR_MPC_Global>())
    {
        Params->Mappings.Remove(TargetParam);
    }
}

void UUR_MPC_Global::GetMappedParameters(FName SourceParam, TSet<FName>& OutParams)
{
    if (const auto Params = GetDefault<UUR_MPC_Global>())
    {
        for (const TPair<FName, FName>& Pair : Params->Mappings)
        {
            if (Pair.Value.IsEqual(SourceParam))
            {
                OutParams.Add(Pair.Key);
            }
        }
    }
}

FName UUR_MPC_Global::GetMapping(FName TargetParam)
{
    if (const auto Params = GetDefault<UUR_MPC_Global>())
    {
        if (const FName* SourceParam = Params->Mappings.Find(TargetParam))
        {
            return *SourceParam;
        }
    }
    return NAME_None;
}


UMaterialParameterCollection* UUR_MPC_Global::GetCollection(UObject* WorldContext)
{
    if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
    {
        if (AUR_PlayerController* PC = World->GetFirstPlayerController<AUR_PlayerController>())
        {
            return PC->MPC_GlobalGame;
        }
    }
    return NULL;
}
