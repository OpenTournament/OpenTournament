// Copyright (c) Open Tournament Games, All Rights Reserved.

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "UR_NumberPopComponent_NiagaraText.h"

#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"

#include "UR_LogChannels.h"
#include "Feedback/NumberPops/UR_DamagePopStyleNiagara.h"
#include "Feedback/NumberPops/UR_NumberPopComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UR_NumberPopComponent_NiagaraText)

/////////////////////////////////////////////////////////////////////////////////////////////////

UUR_NumberPopComponent_NiagaraText::UUR_NumberPopComponent_NiagaraText(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{}

void UUR_NumberPopComponent_NiagaraText::AddNumberPop(const FGameNumberPopRequest& NewRequest)
{
    int32 LocalDamage = NewRequest.NumberToDisplay;

    // Change Damage to negative to differentiate Critical vs Normal hit
    if (NewRequest.bIsCriticalDamage)
    {
        LocalDamage *= -1;
    }

    //Add a NiagaraComponent if we don't already have one
    if (!NiagaraComp)
    {
        NiagaraComp = NewObject<UNiagaraComponent>(GetOwner());
        if (Style != nullptr)
        {
            NiagaraComp->SetAsset(Style->TextNiagara);
            NiagaraComp->bAutoActivate = false;
        }
        NiagaraComp->SetupAttachment(nullptr);
        check(NiagaraComp);
        NiagaraComp->RegisterComponent();
    }


    NiagaraComp->Activate(false);
    NiagaraComp->SetWorldLocation(NewRequest.WorldLocation);

    UE_LOG(LogGame, Log, TEXT("DamageHit location : %s"), *(NewRequest.WorldLocation.ToString()));

    //Add Damage information to the current Niagara list - Damage information is packed inside a FVector4 where XYZ = Position, W = Damage
    TArray<FVector4> DamageList = UNiagaraDataInterfaceArrayFunctionLibrary::GetNiagaraArrayVector4(NiagaraComp, Style->NiagaraArrayName);
    DamageList.Add(FVector4(NewRequest.WorldLocation.X, NewRequest.WorldLocation.Y, NewRequest.WorldLocation.Z, LocalDamage));
    UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector4(NiagaraComp, Style->NiagaraArrayName, DamageList);
}
