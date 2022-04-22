// Copyright (c) 2019-2020 Open Tournament Project, All Rights Reserved.

#include "UR_MPC_Global.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Engine/Engine.h"
#include "Materials/MaterialParameterCollection.h"

#include "UR_BasePlayerController.h"


UUR_MPC_Global* UUR_MPC_Global::Get(UObject* WorldContext, bool bFallbackToCDO)
{
    // Try to work with an instance rather than CDO
    // Modifying CDO affects all clients in PIE multiplayer which causes issues

    auto PC = Cast<AUR_BasePlayerController>(WorldContext);
    if (!PC)
    {
        PC = Cast<AUR_BasePlayerController>(UGameplayStatics::GetPlayerController(WorldContext, 0));
    }
    if (PC)
    {
        if (!PC->UR_MPC_Global_Ref && PC->IsLocalPlayerController())
        {
            PC->UR_MPC_Global_Ref = NewObject<UUR_MPC_Global>(PC);
        }
        if (PC->UR_MPC_Global_Ref)
        {
            return PC->UR_MPC_Global_Ref;
        }
    }

    // Provide default as fallback for access to parameters in BP construct/design
    return bFallbackToCDO ? GetMutableDefault<UUR_MPC_Global>() : NULL;
}


void UUR_MPC_Global::SetScalar(UObject* WorldContext, FName Param, float Value)
{
    UKismetMaterialLibrary::SetScalarParameterValue(WorldContext, GetCollection(WorldContext), Param, Value);

    // Cascade update to mapped parameters
    TSet<FName> MappedParams;
    GetMappedParameters(WorldContext, Param, MappedParams);
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
    GetMappedParameters(WorldContext, Param, MappedParams);
    for (FName& Target : MappedParams)
    {
        SetVector(WorldContext, Target, Value);
    }
}

FLinearColor UUR_MPC_Global::GetVector(UObject* WorldContext, FName Param)
{
    return UKismetMaterialLibrary::GetVectorParameterValue(WorldContext, GetCollection(WorldContext), Param);
}


FPaniniMaterialParameters UUR_MPC_Global::GetPaniniParameters(UObject* WorldContext)
{
    FPaniniMaterialParameters* Params;
    auto Instance = Get(WorldContext, false);
    if (Instance)
    {
        Params = &Instance->PaniniValues;
    }
    else
    {
        static FPaniniMaterialParameters NewParams = FPaniniMaterialParameters();
        NewParams.bInitialized = false;
        Params = &NewParams;
    }
    if (!Params->bInitialized)
    {
        auto Collection = GetCollection(WorldContext);
        Params->DistanceBias = UKismetMaterialLibrary::GetScalarParameterValue(WorldContext, Collection, FName(TEXT("Panini.DistanceBias")));
        Params->DistanceNormalize = UKismetMaterialLibrary::GetScalarParameterValue(WorldContext, Collection, FName(TEXT("Panini.DistanceNormalize")));
        Params->PushMin = UKismetMaterialLibrary::GetScalarParameterValue(WorldContext, Collection, FName(TEXT("Panini.PushMin")));
        Params->PushMax = UKismetMaterialLibrary::GetScalarParameterValue(WorldContext, Collection, FName(TEXT("Panini.PushMax")));
        Params->Scale = UKismetMaterialLibrary::GetScalarParameterValue(WorldContext, Collection, FName(TEXT("Panini.Scale")));
        Params->Depth = UKismetMaterialLibrary::GetScalarParameterValue(WorldContext, Collection, FName(TEXT("Panini.Depth")));
        Params->Skew = UKismetMaterialLibrary::GetScalarParameterValue(WorldContext, Collection, FName(TEXT("Panini.Skew")));
        Params->Projection = UKismetMaterialLibrary::GetScalarParameterValue(WorldContext, Collection, FName(TEXT("Panini.Projection")));
        Params->bInitialized = true;
    }
    return *Params;
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
            GetMappedParameters(WorldContext, Target, NewSet);
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
    auto Params = UUR_MPC_Global::Get(WorldContext, false);
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
    if (auto Params = UUR_MPC_Global::Get(WorldContext, false))
    {
        Params->Mappings.Remove(TargetParam);
    }
}

void UUR_MPC_Global::GetMappedParameters(UObject* WorldContext, FName SourceParam, TSet<FName>& OutParams)
{
    if (const auto Params = UUR_MPC_Global::Get(WorldContext, false))
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

FName UUR_MPC_Global::GetMapping(UObject* WorldContext, FName TargetParam)
{
    if (const auto Params = UUR_MPC_Global::Get(WorldContext, false))
    {
        if (const FName* SourceParam = Params->Mappings.Find(TargetParam))
        {
            return *SourceParam;
        }
    }
    return NAME_None;
}


UMaterialParameterCollection* UUR_MPC_Global::GetCollection(const UObject* WorldContext)
{
    //NOTE: When called from BasePC::PostInitializeComponents, the World has not registered the PC yet.
    // Provide workaround when passing PC as WorldContext.
    if (const auto PC = Cast<AUR_BasePlayerController>(WorldContext))
    {
        return PC->MPC_GlobalGame;
    }
    if (const auto PC = Cast<AUR_BasePlayerController>(UGameplayStatics::GetPlayerController(WorldContext, 0)))
    {
        return PC->MPC_GlobalGame;
    }
    UE_LOG(LogTemp, Warning, TEXT("MPCGlobal: GetCollection() return NULL"));
    return NULL;
}
